#ifndef POLI_H
#define POLI_H

typedef struct {
    int nVars;
    int nMon;
    int nFact;
    double *vCoef;
    int *vOff;
    int *vIdVar;
    int *vExp;
} Poli;

double powi(double x, int e);
void gradSec(const Poli *p, const double *vX, double *vGrad);
double evalSec(const Poli *p, const double *vX);
int parsePoli(const char *txt, char **vNombres, int nVars, Poli *p);
void liberaPoli(Poli *p);

#endif
