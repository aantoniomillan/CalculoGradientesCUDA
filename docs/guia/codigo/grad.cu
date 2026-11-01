#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cuda_runtime.h>

#define KMAX 32
#define CHECK(x) do { cudaError_t err = (x); if (err != cudaSuccess) { printf("CUDA %s linea %d\n", cudaGetErrorString(err), __LINE__); exit(1); } } while (0)

__device__ double powiDev(double x, int e) {
    double r = 1.0;
    while (e > 0) {
        if (e & 1) r *= x;
        x *= x;
        e >>= 1;
    }
    return r;
}

__global__ void kMonoAtom(int nMon, const double *__restrict__ vCoef, const int *__restrict__ vOff,
                          const int *__restrict__ vIdVar, const int *__restrict__ vExp,
                          const double *__restrict__ vX, double *vGrad) {
    int m = blockIdx.x * blockDim.x + threadIdx.x;
    if (m >= nMon) return;
    int ini = vOff[m], k = vOff[m + 1] - ini;
    double vPre[KMAX];
    double acum = vCoef[m];
    for (int t = 0; t < k; t++) {
        double x = __ldg(&vX[vIdVar[ini + t]]);
        int e = vExp[ini + t];
        double base = powiDev(x, e - 1);
        vPre[t] = acum * e * base;
        acum *= base * x;
    }
    double suf = 1.0;
    for (int t = k - 1; t >= 0; t--) {
        int v = vIdVar[ini + t];
        atomicAdd(&vGrad[v], vPre[t] * suf);
        suf *= powiDev(__ldg(&vX[v]), vExp[ini + t]);
    }
}

__global__ void kContrib(int nMon, const double *__restrict__ vCoef, const int *__restrict__ vOff,
                         const int *__restrict__ vIdVar, const int *__restrict__ vExp,
                         const double *__restrict__ vX, double *vContrib) {
    int m = blockIdx.x * blockDim.x + threadIdx.x;
    if (m >= nMon) return;
    int ini = vOff[m], k = vOff[m + 1] - ini;
    double acum = vCoef[m];
    for (int t = 0; t < k; t++) {
        double x = __ldg(&vX[vIdVar[ini + t]]);
        double base = powiDev(x, vExp[ini + t] - 1);
        vContrib[ini + t] = acum * vExp[ini + t] * base;
        acum *= base * x;
    }
    double suf = 1.0;
    for (int t = k - 1; t >= 0; t--) {
        vContrib[ini + t] *= suf;
        suf *= powiDev(__ldg(&vX[vIdVar[ini + t]]), vExp[ini + t]);
    }
}

__global__ void kSumaVar(int nVars, const int *__restrict__ vOffVar, const int *__restrict__ vIdFact,
                         const double *__restrict__ vContrib, double *vGrad) {
    int warp = (blockIdx.x * blockDim.x + threadIdx.x) / 32;
    int lane = threadIdx.x % 32;
    if (warp >= nVars) return;
    double s = 0.0;
    for (int j = vOffVar[warp] + lane; j < vOffVar[warp + 1]; j += 32) s += vContrib[vIdFact[j]];
    for (int d = 16; d > 0; d >>= 1) s += __shfl_down_sync(0xffffffff, s, d);
    if (lane == 0) vGrad[warp] = s;
}

int main(int argc, char **argv) {
    int nVars = argc > 1 ? atoi(argv[1]) : 1000;
    int nMon = argc > 2 ? atoi(argv[2]) : 1000000;
    int tamBloque = argc > 3 ? atoi(argv[3]) : 256;
    srand(0);
    std::vector<double> vCoef(nMon), vX(nVars), vGrad(nVars);
    std::vector<int> vOff(nMon + 1), vIdVar, vExp;
    for (int m = 0; m < nMon; m++) {
        vCoef[m] = 1 + rand() % 10;
        int k = 1 + rand() % 16;
        for (int t = 0; t < k; t++) {
            vIdVar.push_back(rand() % nVars);
            vExp.push_back(1 + rand() % 5);
        }
        vOff[m + 1] = (int)vIdVar.size();
    }
    for (int i = 0; i < nVars; i++) vX[i] = 0.5 + (double)rand() / RAND_MAX;
    int nFact = (int)vIdVar.size();
    double *dCoef, *dX, *dGrad;
    int *dOff, *dIdVar, *dExp;
    cudaEvent_t ev0, ev1, ev2, ev3;
    CHECK(cudaEventCreate(&ev0)); CHECK(cudaEventCreate(&ev1));
    CHECK(cudaEventCreate(&ev2)); CHECK(cudaEventCreate(&ev3));
    CHECK(cudaMalloc(&dCoef, sizeof(double) * nMon));
    CHECK(cudaMalloc(&dOff, sizeof(int) * (nMon + 1)));
    CHECK(cudaMalloc(&dIdVar, sizeof(int) * nFact));
    CHECK(cudaMalloc(&dExp, sizeof(int) * nFact));
    CHECK(cudaMalloc(&dX, sizeof(double) * nVars));
    CHECK(cudaMalloc(&dGrad, sizeof(double) * nVars));
    CHECK(cudaEventRecord(ev0));
    CHECK(cudaMemcpy(dCoef, vCoef.data(), sizeof(double) * nMon, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(dOff, vOff.data(), sizeof(int) * (nMon + 1), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(dIdVar, vIdVar.data(), sizeof(int) * nFact, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(dExp, vExp.data(), sizeof(int) * nFact, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(dX, vX.data(), sizeof(double) * nVars, cudaMemcpyHostToDevice));
    CHECK(cudaMemset(dGrad, 0, sizeof(double) * nVars));
    CHECK(cudaEventRecord(ev1));
    int nBloques = (nMon + tamBloque - 1) / tamBloque;
    kMonoAtom<<<nBloques, tamBloque>>>(nMon, dCoef, dOff, dIdVar, dExp, dX, dGrad);
    CHECK(cudaGetLastError());
    CHECK(cudaEventRecord(ev2));
    CHECK(cudaMemcpy(vGrad.data(), dGrad, sizeof(double) * nVars, cudaMemcpyDeviceToHost));
    CHECK(cudaEventRecord(ev3));
    CHECK(cudaEventSynchronize(ev3));
    float tH2D, tKer, tD2H;
    CHECK(cudaEventElapsedTime(&tH2D, ev0, ev1));
    CHECK(cudaEventElapsedTime(&tKer, ev1, ev2));
    CHECK(cudaEventElapsedTime(&tD2H, ev2, ev3));
    printf("nFact %d bloque %d\n", nFact, tamBloque);
    printf("H2D %.3f ms kernel %.3f ms D2H %.3f ms total %.3f ms\n", tH2D, tKer, tD2H, tH2D + tKer + tD2H);
    printf("grad[0] %.10e\n", vGrad[0]);
    cudaFree(dCoef); cudaFree(dOff); cudaFree(dIdVar); cudaFree(dExp); cudaFree(dX); cudaFree(dGrad);
    return 0;
}
