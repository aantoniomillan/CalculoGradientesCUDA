#include <cstdio>
#include <cmath>
#include <cuda_runtime.h>

const int nElem = 1 << 24;
const int nRep = 64;

__global__ void kSinD(const float* x, float* y, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    double v = x[i];
    double acc = 0.0;
    for (int r = 0; r < nRep; r++) acc += sin(v + r);
    y[i] = (float)acc;
}

__global__ void kSinF(const float* x, float* y, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    float v = x[i];
    float acc = 0.0f;
    for (int r = 0; r < nRep; r++) acc += sinf(v + r);
    y[i] = acc;
}

__global__ void kSinI(const float* x, float* y, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    float v = x[i];
    float acc = 0.0f;
    for (int r = 0; r < nRep; r++) acc += __sinf(v + r);
    y[i] = acc;
}

__global__ void kErr(const float* x, float* eF, float* eI, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    double ref = sin((double)x[i]);
    eF[i] = (float)fabs((double)sinf(x[i]) - ref);
    eI[i] = (float)fabs((double)__sinf(x[i]) - ref);
}

float medir(void (*k)(const float*, float*, int), const float* dX, float* dY, int bloques, int hilos) {
    cudaEvent_t ini, fin;
    cudaEventCreate(&ini);
    cudaEventCreate(&fin);
    k<<<bloques, hilos>>>(dX, dY, nElem);
    cudaEventRecord(ini);
    k<<<bloques, hilos>>>(dX, dY, nElem);
    cudaEventRecord(fin);
    cudaEventSynchronize(fin);
    float ms = 0.0f;
    cudaEventElapsedTime(&ms, ini, fin);
    cudaEventDestroy(ini);
    cudaEventDestroy(fin);
    return ms;
}

void maxErr(float rango, float* hX, float* dX, float* dEF, float* dEI, float* hEF, float* hEI, int bloques, int hilos) {
    for (int i = 0; i < nElem; i++) hX[i] = -rango + 2.0f * rango * (float)i / nElem;
    cudaMemcpy(dX, hX, nElem * sizeof(float), cudaMemcpyHostToDevice);
    kErr<<<bloques, hilos>>>(dX, dEF, dEI, nElem);
    cudaMemcpy(hEF, dEF, nElem * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(hEI, dEI, nElem * sizeof(float), cudaMemcpyDeviceToHost);
    float mF = 0.0f, mI = 0.0f;
    for (int i = 0; i < nElem; i++) {
        if (hEF[i] > mF) mF = hEF[i];
        if (hEI[i] > mI) mI = hEI[i];
    }
    printf("Rango +-%g  error max sinf: %e  error max __sinf: %e\n", rango, mF, mI);
}

int main() {
    int hilos = 256;
    int bloques = (nElem + hilos - 1) / hilos;
    float* hX = new float[nElem];
    float* hEF = new float[nElem];
    float* hEI = new float[nElem];
    for (int i = 0; i < nElem; i++) hX[i] = (float)i / nElem * 6.2831853f;
    float *dX, *dY, *dEF, *dEI;
    cudaMalloc(&dX, nElem * sizeof(float));
    cudaMalloc(&dY, nElem * sizeof(float));
    cudaMalloc(&dEF, nElem * sizeof(float));
    cudaMalloc(&dEI, nElem * sizeof(float));
    cudaMemcpy(dX, hX, nElem * sizeof(float), cudaMemcpyHostToDevice);
    float tD = medir(kSinD, dX, dY, bloques, hilos);
    float tF = medir(kSinF, dX, dY, bloques, hilos);
    float tI = medir(kSinI, dX, dY, bloques, hilos);
    printf("sin (double): %.3f ms\n", tD);
    printf("sinf: %.3f ms\n", tF);
    printf("__sinf: %.3f ms\n", tI);
    printf("Aceleracion __sinf vs sinf: %.2fx\n", tF / tI);
    printf("Aceleracion __sinf vs sin: %.2fx\n", tD / tI);
    maxErr(3.14159265f, hX, dX, dEF, dEI, hEF, hEI, bloques, hilos);
    maxErr(100.0f, hX, dX, dEF, dEI, hEF, hEI, bloques, hilos);
    maxErr(10000.0f, hX, dX, dEF, dEI, hEF, hEI, bloques, hilos);
    cudaFree(dX);
    cudaFree(dY);
    cudaFree(dEF);
    cudaFree(dEI);
    delete[] hX;
    delete[] hEF;
    delete[] hEI;
    return 0;
}
