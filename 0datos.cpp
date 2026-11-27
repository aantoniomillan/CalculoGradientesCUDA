#include "0datos.h"

#include <cstdio>
#include <iostream>
#include <random>
#include <string>

/*==============================
    FUNCIONES DE CLASE
==============================*/

void GestorDatos::avisar() const {
    cout << "\n AVISO: puedes ajustar los datos como quieras, pero cuidado con "
            "\n el tiempo y la memoria. Con tamanos muy grandes la ejecucion"
            "\n puede tardar demasiado o quedarse sin memoria, sobre todo"
            "\n en la version secuencial. Recomiendo empezar con los valores "
            "\n por defecto y subir poco a poco."

            "\n NOTA: El modo throughput repite la ejecucion durante un tiempo fijo y "
            "\n cuenta cuantas veces termina. Sirve para comparar variantes, pero pierde "
            "\n algo de eficiencia porque un hilo se dedica a cronometrar.\n";
}

void GestorDatos::menu() {
    int op = -1;
    while (op != 0) {
        cout << "\nDatos"
                "\n1-Cargar fichero"
                "\n2-Generar"
                "\n3-Ajustar parametros"
                "\n4-Guardar fichero"
                "\n5-Mostrar"
                "\n6-Modo throughput: " << (throughput ? "si" : "no") <<
                "\n0-Volver"
                "\nIntroduce el numero de una opcion: ";
        if (!(cin >> op)) return;
        switch (op) {
            case 1: cargar();      break;
            case 2: generar();     break;
            case 3: ajustar();     break;
            case 4: guardar();     break;
            case 5: mostrar();     break;
            case 6: cambiarModo(); break;
        }
    }
}

void GestorDatos::ajustar() {
    semilla = pedir("Semilla", semilla, 0);
    nVar    = pedir("Variables", nVar, 1);
    nMon    = pedir("Monomios", nMon, 1);
    nPun    = pedir("Puntos", nPun, 1);
}

uint32_t GestorDatos::pedir(const char* texto, uint32_t actual, uint32_t minimo) const {
    string s;
    while (true) {
        cout << texto << " (" << actual << "): ";
        if (!(cin >> s)) return actual;
        bool valido = !s.empty() && s.size()<=10;

        for (char c : s)
            if (c < '0' || c > '9') valido = false;

        if (valido) {
            unsigned long long v = stoull(s);
            if (v >= minimo && v <= UINT32_MAX) return uint32_t(v);
        }
        cout << "Introduce un numero entero positivo\n";
    }
}

void GestorDatos::preparar() {
    Datos tmp;
    if (!leerF(tmp)) {
        crearF(tmp);
        escribirF(tmp);
    }
    if (!leerX(tmp)) {
        crearX(tmp);
        escribirX(tmp);
    }
    df = tmp;
}

void GestorDatos::mostrar() const {
    uint32_t maxExp = 0;
    uint64_t sumExp = 0;
    for (uint8_t e : df.exps) {
        if (e > maxExp) maxExp = e;
        sumExp += e;
    }
    cout << "Variables: " << df.nVar
         << "\nMonomios: " << df.nMon
         << "\nPuntos: " << df.nPun
         << "\nMax exponente: " << maxExp << "\n";
    if (!df.exps.empty())
        cout << "Promedio exponentes: " << double(sumExp) / df.exps.size() << "\n";

    for (uint32_t m = 0; m < df.nMon && m < 5; m++) {
        if (m > 0) cout << " + ";
        cout << df.coefs[m];
        for (uint32_t i = df.mon[m]; i < df.mon[m + 1]; i++)
            cout << "*x" << df.vars[i] << "^" << int(df.exps[i]);
    }
    if (df.nMon > 5) cout << " + ...";
    cout << "\n";
}

/*==============================
    FUNCIONES DE GENERACION
==============================*/

void GestorDatos::generar() {
    crearF(df);
    crearX(df);
    cout << "Generado\n";
}


/*==============================
    FUNCIONES DE FICHEROS
    Generadas por IA
==============================*/

void GestorDatos::rutaF(char* ruta) const {
    snprintf(ruta, 128, "f_%u_%u_%u.bin", nMon, nVar, semilla);
}

void GestorDatos::rutaX(char* ruta) const {
    snprintf(ruta, 128, "x_%u_%u_%u.bin", nPun, nVar, semilla);
}

void GestorDatos::cargar() {
    Datos tmp;
    bool ok = leerF(tmp) && leerX(tmp);
    if (ok) df = tmp;
    cout << (ok ? "Cargado\n" : "No se pudo cargar\n");
}

void GestorDatos::guardar() const {
    if (df.nVar != nVar || df.nMon != nMon || df.nPun != nPun) {
        cout << "Los datos actuales no coinciden con los parametros. Genera o carga primero\n";
        return;
    }
    bool ok = escribirF(df) && escribirX(df);
    cout << (ok ? "Guardado\n" : "No se pudo guardar\n");
}

void GestorDatos::crearF(Datos& d) const {
    mt19937 gen(semilla);
    uint32_t kMax = nVar < 16 ? nVar : 16;
    uniform_int_distribution<int> dCoef(1, 9);
    uniform_int_distribution<uint32_t> dK(0, kMax);
    uniform_int_distribution<uint32_t> dVar(0, nVar - 1);
    uniform_int_distribution<int> dExp(1, 5);

    d.nVar = nVar;
    d.nMon = nMon;
    d.coefs.resize(nMon);
    d.mon.resize(size_t(nMon) + 1);
    d.vars.clear();
    d.exps.clear();
    d.vars.reserve(size_t(nMon) * kMax / 2);
    d.exps.reserve(size_t(nMon) * kMax / 2);

    d.mon[0] = 0;
    for (uint32_t m = 0; m < nMon; m++) {
        d.coefs[m] = dCoef(gen);
        uint32_t k = dK(gen);
        for (uint32_t t = 0; t < k; t++) {
            uint32_t v;
            bool repetida;
            do {
                v = dVar(gen);
                repetida = false;
                for (size_t i = d.mon[m]; i < d.vars.size(); i++)
                    if (d.vars[i] == v) repetida = true;
            } while (repetida);
            d.vars.push_back(v);
            d.exps.push_back(uint8_t(dExp(gen)));
        }
        d.mon[m + 1] = uint32_t(d.vars.size());
    }
}

void GestorDatos::crearX(Datos& d) const {
    mt19937 gen(semilla);
    uniform_real_distribution<double> dX(0.5, 1.5);

    d.nVar = nVar;
    d.nPun = nPun;
    d.x.resize(size_t(nPun) * nVar);
    for (double& v : d.x) v = dX(gen);
}

bool GestorDatos::leerF(Datos& d) const {
    char ruta[128];
    rutaF(ruta);
    FILE* f = fopen(ruta, "rb");
    if (f == nullptr) return false;

    uint32_t cab[3];
    if (fread(cab, sizeof(uint32_t), 3, f) != 3 || cab[0] != nVar || cab[1] != nMon) {
        fclose(f);
        return false;
    }

    d.nVar = cab[0];
    d.nMon = cab[1];
    d.coefs.resize(cab[1]);
    d.mon.resize(size_t(cab[1]) + 1);
    d.vars.resize(cab[2]);
    d.exps.resize(cab[2]);

    size_t esperado = d.coefs.size() + d.mon.size() + d.vars.size() + d.exps.size();
    size_t n = fread(d.coefs.data(), sizeof(double), d.coefs.size(), f);
    n += fread(d.mon.data(), sizeof(uint32_t), d.mon.size(), f);
    n += fread(d.vars.data(), sizeof(uint32_t), d.vars.size(), f);
    n += fread(d.exps.data(), sizeof(uint8_t), d.exps.size(), f);
    fclose(f);
    return n == esperado;
}

bool GestorDatos::leerX(Datos& d) const {
    char ruta[128];
    rutaX(ruta);
    FILE* f = fopen(ruta, "rb");
    if (f == nullptr) return false;

    uint32_t cab[2];
    if (fread(cab, sizeof(uint32_t), 2, f) != 2 || cab[0] != nVar || cab[1] != nPun) {
        fclose(f);
        return false;
    }

    d.nVar = cab[0];
    d.nPun = cab[1];
    d.x.resize(size_t(cab[1]) * cab[0]);

    size_t n = fread(d.x.data(), sizeof(double), d.x.size(), f);
    fclose(f);
    return n == d.x.size();
}

bool GestorDatos::escribirF(const Datos& d) const {
    char ruta[128];
    rutaF(ruta);
    FILE* f = fopen(ruta, "wb");
    if (f == nullptr) return false;

    uint32_t cab[3] = {d.nVar, d.nMon, uint32_t(d.vars.size())};
    size_t esperado = 3 + d.coefs.size() + d.mon.size() + d.vars.size() + d.exps.size();
    size_t n = fwrite(cab, sizeof(uint32_t), 3, f);
    n += fwrite(d.coefs.data(), sizeof(double), d.coefs.size(), f);
    n += fwrite(d.mon.data(), sizeof(uint32_t), d.mon.size(), f);
    n += fwrite(d.vars.data(), sizeof(uint32_t), d.vars.size(), f);
    n += fwrite(d.exps.data(), sizeof(uint8_t), d.exps.size(), f);
    fclose(f);
    return n == esperado;
}

bool GestorDatos::escribirX(const Datos& d) const {
    char ruta[128];
    rutaX(ruta);
    FILE* f = fopen(ruta, "wb");
    if (f == nullptr) return false;

    uint32_t cab[2] = {d.nVar, d.nPun};
    size_t n = fwrite(cab, sizeof(uint32_t), 2, f);
    n += fwrite(d.x.data(), sizeof(double), d.x.size(), f);
    fclose(f);
    return n == 2 + d.x.size();
}

void GestorDatos::regTiempo(const char* metodo, double tiempo) const {
    char ruta[64];
    snprintf(ruta, sizeof(ruta), "%s.csv", metodo);

    FILE* f = fopen(ruta, "r");
    bool nuevo = (f == nullptr);
    if (f != nullptr) fclose(f);

    f = fopen(ruta, "a");
    if (f == nullptr) return;
    if (nuevo) fprintf(f, "semilla,nVar,nMon,nPun,tiempo\n");
    fprintf(f, "%u,%u,%u,%u,%.6f\n", semilla, df.nVar, df.nMon, df.nPun, tiempo);
    fclose(f);
}
