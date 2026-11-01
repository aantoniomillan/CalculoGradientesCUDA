#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "poli.h"

int main(void) {
    FILE *f = fopen("funcion.txt", "r");
    FILE *fRef = fopen("ref.txt", "r");
    if (!f || !fRef) { printf("Faltan ficheros\n"); return 1; }
    size_t tam = 1 << 24;
    char *linea = malloc(tam);
    fgets(linea, (int)tam, f);
    int nVars = 0;
    char **vNombres = malloc(sizeof(char *) * 100000);
    for (char *tok = strtok(linea, " \n"); tok; tok = strtok(NULL, " \n")) vNombres[nVars++] = strdup(tok);
    fgets(linea, (int)tam, f);
    Poli p;
    if (parsePoli(linea, vNombres, nVars, &p) != 0) { printf("Error de parseo\n"); return 1; }
    double *vX = malloc(sizeof(double) * nVars), *vGrad = malloc(sizeof(double) * nVars);
    for (int i = 0; i < nVars; i++) fscanf(f, "%lf", &vX[i]);
    gradSec(&p, vX, vGrad);
    double rtol = 1e-12, maxErr = 0.0;
    for (int i = 0; i < nVars; i++) {
        double ref;
        fscanf(fRef, "%lf", &ref);
        double err = fabs(vGrad[i] - ref) / fmax(1.0, fabs(ref));
        if (err > maxErr) maxErr = err;
    }
    printf("nMon %d nFact %d errMax %.3e %s\n", p.nMon, p.nFact, maxErr, maxErr <= rtol ? "OK" : "FALLO");
    return 0;
}
