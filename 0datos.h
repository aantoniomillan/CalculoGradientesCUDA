#pragma once

#include <cstdint>
#include <vector>

using namespace std;

struct Datos {
    uint32_t nVar = 0;
    uint32_t nMon = 0;
    uint32_t nPun = 0;
    vector<double> coefs;
    vector<uint8_t> exps;
    vector<uint32_t> vars;
    vector<uint32_t> mon;
    vector<double> x;
};

class GestorDatos {
    private:
        Datos df;
        uint32_t semilla = 67;
        uint32_t nVar = 1000;
        uint32_t nMon = 100000;
        uint32_t nPun = 1;
        bool throughput = false;

        void rutaF(char* ruta) const;
        void rutaX(char* ruta) const;
        uint32_t pedir(const char* texto, uint32_t actual, uint32_t minimo) const;
        void cambiarModo() { throughput = !throughput; }

        void crearF(Datos& d) const;
        void crearX(Datos& d) const;
        bool leerF(Datos& d) const;
        bool leerX(Datos& d) const;
        bool escribirF(const Datos& d) const;
        bool escribirX(const Datos& d) const;

    public:
        void avisar() const;
        void preparar();
        void menu();
        void ajustar();
        void generar();
        void cargar();
        void guardar() const;
        void mostrar() const;
        void regTiempo(const char* metodo, double tiempo) const;

        bool modo() const { return throughput; }
        const Datos& datos() const { return df; }
};
