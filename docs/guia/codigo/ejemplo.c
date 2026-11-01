#include <stdio.h>
#include "poli.h"

int main(void) {
    char *vNombres[] = {"x", "y", "z"};
    double vX[] = {1.0, 2.0, 3.0};
    double vGrad[3];
    Poli p;
    if (parsePoli("3xy + 4yz + 2x + 5", vNombres, 3, &p) != 0) {
        printf("Error de parseo\n");
        return 1;
    }
    gradSec(&p, vX, vGrad);
    printf("f = %g\n", evalSec(&p, vX));
    printf("grad = (%g, %g, %g)\n", vGrad[0], vGrad[1], vGrad[2]);
    liberaPoli(&p);
    return 0;
}
