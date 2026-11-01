#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "poli.h"

void generaPoli(Poli *p, int nVars, int nMon, int kMax, int eMax, unsigned semilla) {
    srand(semilla);
    p->nVars = nVars;
    p->nMon = nMon;
    p->vCoef = malloc(sizeof(double) * nMon);
    p->vOff = malloc(sizeof(int) * (nMon + 1));
    p->vIdVar = malloc(sizeof(int) * nMon * kMax);
    p->vExp = malloc(sizeof(int) * nMon * kMax);
    int nFact = 0;
    p->vOff[0] = 0;
    for (int m = 0; m < nMon; m++) {
        p->vCoef[m] = 1 + rand() % 10;
        int k = 1 + rand() % kMax;
        for (int t = 0; t < k; t++) {
            p->vIdVar[nFact] = rand() % nVars;
            p->vExp[nFact] = 1 + rand() % eMax;
            nFact++;
        }
        p->vOff[m + 1] = nFact;
    }
    p->nFact = nFact;
}

void gradRed(const Poli *p, const double *vX, double *vGrad) {
    int n = p->nVars;
    memset(vGrad, 0, sizeof(double) * n);
    #pragma omp parallel for schedule(dynamic, 256) reduction(+ : vGrad[:n])
    for (int m = 0; m < p->nMon; m++) {
        int ini = p->vOff[m], fin = p->vOff[m + 1];
        for (int t = ini; t < fin; t++) {
            double d = p->vCoef[m] * p->vExp[t] * powi(vX[p->vIdVar[t]], p->vExp[t] - 1);
            for (int s = ini; s < fin; s++)
                if (s != t) d *= powi(vX[p->vIdVar[s]], p->vExp[s]);
            vGrad[p->vIdVar[t]] += d;
        }
    }
}

void gradAtom(const Poli *p, const double *vX, double *vGrad) {
    memset(vGrad, 0, sizeof(double) * p->nVars);
    #pragma omp parallel for schedule(dynamic, 256)
    for (int m = 0; m < p->nMon; m++) {
        int ini = p->vOff[m], fin = p->vOff[m + 1];
        for (int t = ini; t < fin; t++) {
            double d = p->vCoef[m] * p->vExp[t] * powi(vX[p->vIdVar[t]], p->vExp[t] - 1);
            for (int s = ini; s < fin; s++)
                if (s != t) d *= powi(vX[p->vIdVar[s]], p->vExp[s]);
            #pragma omp atomic
            vGrad[p->vIdVar[t]] += d;
        }
    }
}

void transpone(const Poli *p, int *vOffVar, int *vIdFact) {
    int n = p->nVars;
    memset(vOffVar, 0, sizeof(int) * (n + 1));
    for (int t = 0; t < p->nFact; t++) vOffVar[p->vIdVar[t] + 1]++;
    for (int v = 0; v < n; v++) vOffVar[v + 1] += vOffVar[v];
    int *vPos = malloc(sizeof(int) * n);
    memcpy(vPos, vOffVar, sizeof(int) * n);
    for (int t = 0; t < p->nFact; t++) vIdFact[vPos[p->vIdVar[t]]++] = t;
    free(vPos);
}

void gradDosFases(const Poli *p, const double *vX, const int *vOffVar, const int *vIdFact,
                  double *vContrib, double *vGrad) {
    #pragma omp parallel
    {
        double vPre[64];
        #pragma omp for schedule(dynamic, 256)
        for (int m = 0; m < p->nMon; m++) {
            int ini = p->vOff[m], k = p->vOff[m + 1] - ini;
            double acum = p->vCoef[m];
            for (int t = 0; t < k; t++) {
                double base = powi(vX[p->vIdVar[ini + t]], p->vExp[ini + t] - 1);
                vPre[t] = acum * p->vExp[ini + t] * base;
                acum *= base * vX[p->vIdVar[ini + t]];
            }
            double suf = 1.0;
            for (int t = k - 1; t >= 0; t--) {
                vContrib[ini + t] = vPre[t] * suf;
                suf *= powi(vX[p->vIdVar[ini + t]], p->vExp[ini + t]);
            }
        }
        #pragma omp for schedule(static)
        for (int v = 0; v < p->nVars; v++) {
            double s = 0.0;
            for (int j = vOffVar[v]; j < vOffVar[v + 1]; j++) s += vContrib[vIdFact[j]];
            vGrad[v] = s;
        }
    }
}

void gradPuntos(const Poli *p, int nPts, const double *mX, double *mGrad) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < nPts; i++)
        gradSec(p, mX + (size_t)i * p->nVars, mGrad + (size_t)i * p->nVars);
}

double errRel(const double *vA, const double *vB, int n) {
    double maxErr = 0.0;
    for (int i = 0; i < n; i++) {
        double den = fmax(fabs(vB[i]), 1e-300);
        double e = fabs(vA[i] - vB[i]) / den;
        if (vB[i] == 0.0) e = fabs(vA[i]);
        if (e > maxErr) maxErr = e;
    }
    return maxErr;
}

int main(int argc, char **argv) {
    int nVars = argc > 1 ? atoi(argv[1]) : 1000;
    int nMon = argc > 2 ? atoi(argv[2]) : 100000;
    int kMax = argc > 3 ? atoi(argv[3]) : 16;
    Poli p;
    generaPoli(&p, nVars, nMon, kMax, 5, 0);
    double *vX = malloc(sizeof(double) * nVars);
    srand(1);
    for (int i = 0; i < nVars; i++) vX[i] = 0.5 + (double)rand() / RAND_MAX;
    double *vRef = malloc(sizeof(double) * nVars);
    double *vGrad = malloc(sizeof(double) * nVars);
    double *vContrib = malloc(sizeof(double) * p.nFact);
    int *vOffVar = malloc(sizeof(int) * (nVars + 1));
    int *vIdFact = malloc(sizeof(int) * p.nFact);
    printf("nVars %d nMon %d nFact %d hilos %d\n", nVars, nMon, p.nFact, omp_get_max_threads());
    double t0 = omp_get_wtime();
    gradSec(&p, vX, vRef);
    printf("secuencial %.6f s\n", omp_get_wtime() - t0);
    t0 = omp_get_wtime();
    gradRed(&p, vX, vGrad);
    printf("reduction %.6f s errRel %.3e\n", omp_get_wtime() - t0, errRel(vGrad, vRef, nVars));
    t0 = omp_get_wtime();
    gradAtom(&p, vX, vGrad);
    printf("atomic %.6f s errRel %.3e\n", omp_get_wtime() - t0, errRel(vGrad, vRef, nVars));
    transpone(&p, vOffVar, vIdFact);
    t0 = omp_get_wtime();
    gradDosFases(&p, vX, vOffVar, vIdFact, vContrib, vGrad);
    printf("dosFases %.6f s errRel %.3e\n", omp_get_wtime() - t0, errRel(vGrad, vRef, nVars));
    liberaPoli(&p);
    free(vX); free(vRef); free(vGrad); free(vContrib); free(vOffVar); free(vIdFact);
    return 0;
}
