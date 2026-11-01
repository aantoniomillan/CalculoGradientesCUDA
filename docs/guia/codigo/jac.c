#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "poli.h"

int main(void) {
    const char *vEc[] = {"3x^2 + yz + z + 5 = 2", "2x + yz^3 + 2x + 2 = 0", "3xz + 8yz + z^2 + 1 = 4"};
    char *vNombres[] = {"x", "y", "z"};
    int nEc = 3, nVars = 3;
    double vX[] = {1.0, 2.0, 3.0};
    double *mJac = malloc(sizeof(double) * nEc * nVars);
    for (int i = 0; i < nEc; i++) {
        char buf[256];
        strcpy(buf, vEc[i]);
        char *igual = strchr(buf, '=');
        double rhs = atof(igual + 1);
        *igual = '\0';
        Poli p;
        parsePoli(buf, vNombres, nVars, &p);
        gradSec(&p, vX, mJac + i * nVars);
        printf("F%d = %g  fila = (%g, %g, %g)\n", i + 1, evalSec(&p, vX) - rhs,
               mJac[i * nVars], mJac[i * nVars + 1], mJac[i * nVars + 2]);
        liberaPoli(&p);
    }
    free(mJac);
    return 0;
}
