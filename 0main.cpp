#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "0datos.h"
#include "1secuencial.h"
#include "2omp.h"

#ifndef SIN_CUDA
#include "3cuda.h"
#endif

#include "4jacobiano.h"
#include "5super.h"

using namespace std;

inline double tiempo(); // inline?
void cabecera();

int menu() {
    cout << "\n1-Secuencial"
            "\n2-OpenMP"
            "\n3-CUDA"
            "\n4-Jacobiano"
            "\n5-Super"
            "\n6-Ajustar Datos"
            "\n0-Salir"
            "\nIntroduce elnumero de una Opcion: ";
    int op;
    return (cin >> op)? op:0;
}


bool ejecutar(int op) {
    switch (op) {
        case 1: return secuencial();
        case 2: return omp();
        #ifndef SIN_CUDA
        case 3: return cuda();
        #endif
        case 4: return jacobiano();
        case 5: return super();
    }
    return false;
}

bool e(int op) {
    for(unsigned int i=0;i<100000;i++)
        for(unsigned int j=0;j<100000;j++)
            op++;

    return true;
}

bool pasar(const Datos&){
    return false;
}

int main() {

    bool (*f[])(const Datos&) = 
        {pasar, secuencial, omp,
        #ifndef SIN_CUDA 
            cuda,
        #else 
            pasar,
        #endif
            jacobiano, super};

    const char* metodos[] = {
        "",
        "secuencial", 
        "omp",
        "cuda",
        "jacobiano", 
        "super"
    };

    GestorDatos gd;
    thread hilo([&gd](){gd.preparar();});  // que?
    
    cabecera();
    gd.avisar();

    for(int op = menu(); op != 0 ; op = menu()) {
        if (hilo.joinable()) hilo.join();
        
        if(op < 0 || op > 6 ) op = 0;
        
        if(op == 6){
            gd.menu(); 
            continue;
        }
    
        double ini = tiempo();
        if(!f[op](gd.datos())) cout << "Opcion no disponible\n"; 
        else{
            double total = tiempo() - ini;
            cout<< "Tiempo: " << total <<" s\n";
            gd.regTiempo(metodos[op], total);
        }
    }

    if (hilo.joinable()) hilo.join();
    return 0;
}


/*
==============================
  FUNCIONES GENERADAS POR IA
  + "analisis"
==============================
*/

inline double tiempo() {
    return chrono::duration<double>(chrono::steady_clock::now().time_since_epoch()).count();
    /*
    Del espacio chrono:
    - steady_clock::now() -> nos da la hora actual (formato time_point)
    - time_since_epoch pasa la hora a duration, en ns
    - duration<double> pasa a s
    - count saca el numero
    */
}

void cabecera() {

    //Establecemos ctes de colores
    const char* cian = "\033[1;36m";
    const char* verde = "\033[1;32m";
    const char* rojo = "\033[1;31m";
    const char* gris = "\033[90m";
    const char* fin = "\033[0m";

    // Cout con R evitamos / como escape y escribir la cadena entre unas "" (con salto de linea hariamos varias)
    cout << cian << R"(
  ____               _ _            _
 / ___|_ __ __ _  __| (_) ___ _ __ | |_ ___  ___
| |  _| '__/ _` |/ _` | |/ _ \ '_ \| __/ _ \/ __|
| |_| | | | (_| | (_| | |  __/ | | | ||  __/\__ \
 \____|_|  \__,_|\__,_|_|\___|_| |_|\__\___||___/
)" << fin;

    cout << gris << "  Calculo paralelo de gradientes | Programacion Paralela\n" << fin
         << "  Autor:  " << verde << "Antonio Millan Rodriguez" << fin << "\n"
         << "  Hilos:  " << verde << thread::hardware_concurrency() << fin << "\n"
#ifdef SIN_CUDA
         << "  CUDA:   " << rojo << "no disponible" << fin << "\n";
#else
         << "  CUDA:   " << verde << "disponible" << fin << "\n";
#endif
}