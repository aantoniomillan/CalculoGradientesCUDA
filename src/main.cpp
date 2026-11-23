#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "datos.h"
#include "variantes/jacobiano.h"
#include "variantes/omp.h"
#include "variantes/secuencial.h"
#include "variantes/super.h"
#ifndef SIN_CUDA
#include "variantes/cuda.h"
#endif

using namespace std;

int leerOp() {
    cout << "\n1 Secuencial\n2 OpenMP\n3 CUDA\n4 Jacobiano\n5 Super\n0 Salir\nOpcion: ";
    int op;
    if (!(cin >> op)) {
        return 0;
    }
    return op;
}

int ejecutarOp(int op) {
    switch (op) {
        case 1: return secuencial();
        case 2: return openMp();
#ifndef SIN_CUDA
        case 3: return cuda();
#endif
        case 4: return jacobiano();
        case 5: return super();
    }
    return -1;
}

int main() {
    const uint32_t semilla = 42;
    thread hilo(generarDatos, semilla);

    int op = leerOp();
    while (op != 0) {
        if (hilo.joinable()) {
            hilo.join();
        }
        auto inicio = chrono::steady_clock::now();
        int resultado = ejecutarOp(op);
        auto fin = chrono::steady_clock::now();
        if (resultado < 0) {
            cout << "Opcion no disponible\n";
        } else {
            chrono::duration<double, milli> tiempo = fin - inicio;
            cout << "Tiempo: " << tiempo.count() << " ms\n";
        }
        op = leerOp();
    }

    if (hilo.joinable()) {
        hilo.join();
    }
    return 0;
}
