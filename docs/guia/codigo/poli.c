#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "poli.h"

#define KMAX 64

double powi(double x, int e) {
    double r = 1.0;
    while (e > 0) {
        if (e & 1) r *= x;
        x *= x;
        e >>= 1;
    }
    return r;
}

double evalSec(const Poli *p, const double *vX) {
    double total = 0.0;
    for (int m = 0; m < p->nMon; m++) {
        double prod = p->vCoef[m];
        for (int t = p->vOff[m]; t < p->vOff[m + 1]; t++)
            prod *= powi(vX[p->vIdVar[t]], p->vExp[t]);
        total += prod;
    }
    return total;
}

void gradSec(const Poli *p, const double *vX, double *vGrad) {
    double vPre[KMAX], vDer[KMAX];
    memset(vGrad, 0, sizeof(double) * p->nVars);
    for (int m = 0; m < p->nMon; m++) {
        int ini = p->vOff[m], k = p->vOff[m + 1] - ini;
        double acum = p->vCoef[m];
        for (int t = 0; t < k; t++) {
            double x = vX[p->vIdVar[ini + t]];
            int e = p->vExp[ini + t];
            double base = powi(x, e - 1);
            vPre[t] = acum;
            vDer[t] = e * base;
            acum *= base * x;
        }
        double suf = 1.0;
        for (int t = k - 1; t >= 0; t--) {
            int v = p->vIdVar[ini + t];
            double x = vX[v];
            int e = p->vExp[ini + t];
            vGrad[v] += vPre[t] * vDer[t] * suf;
            suf *= powi(x, e);
        }
    }
}

static int buscaVar(const char *s, int len, char **vNombres, int nVars) {
    for (int i = 0; i < nVars; i++)
        if ((int)strlen(vNombres[i]) == len && strncmp(vNombres[i], s, len) == 0) return i;
    return -1;
}

int parsePoli(const char *txt, char **vNombres, int nVars, Poli *p) {
    int capMon = 16, capFact = 64;
    p->nVars = nVars;
    p->nMon = 0;
    p->nFact = 0;
    p->vCoef = malloc(sizeof(double) * capMon);
    p->vOff = malloc(sizeof(int) * (capMon + 1));
    p->vIdVar = malloc(sizeof(int) * capFact);
    p->vExp = malloc(sizeof(int) * capFact);
    p->vOff[0] = 0;
    const char *c = txt;
    double signo = 1.0;
    while (*c) {
        while (isspace((unsigned char)*c)) c++;
        if (*c == '+') { signo = 1.0; c++; continue; }
        if (*c == '-') { signo = -1.0; c++; continue; }
        if (*c == '\0') break;
        double coef = 1.0;
        if (isdigit((unsigned char)*c) || *c == '.') {
            char *fin;
            coef = strtod(c, &fin);
            c = fin;
        }
        if (p->nMon == capMon) {
            capMon *= 2;
            p->vCoef = realloc(p->vCoef, sizeof(double) * capMon);
            p->vOff = realloc(p->vOff, sizeof(int) * (capMon + 1));
        }
        int ini = p->nFact;
        while (*c == '*' || isalpha((unsigned char)*c)) {
            if (*c == '*') { c++; continue; }
            const char *s = c++;
            while (isdigit((unsigned char)*c)) c++;
            int v = buscaVar(s, (int)(c - s), vNombres, nVars);
            if (v < 0) return -1;
            int e = 1;
            if (*c == '^') { c++; e = (int)strtol(c, (char **)&c, 10); }
            int t = ini;
            while (t < p->nFact && p->vIdVar[t] != v) t++;
            if (t < p->nFact) { p->vExp[t] += e; continue; }
            if (p->nFact == capFact) {
                capFact *= 2;
                p->vIdVar = realloc(p->vIdVar, sizeof(int) * capFact);
                p->vExp = realloc(p->vExp, sizeof(int) * capFact);
            }
            p->vIdVar[p->nFact] = v;
            p->vExp[p->nFact] = e;
            p->nFact++;
        }
        p->vCoef[p->nMon] = signo * coef;
        p->nMon++;
        p->vOff[p->nMon] = p->nFact;
        signo = 1.0;
    }
    return 0;
}

void liberaPoli(Poli *p) {
    free(p->vCoef);
    free(p->vOff);
    free(p->vIdVar);
    free(p->vExp);
}
