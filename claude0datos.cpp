#include "0datos.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

static const char marca[4] = {'G', 'R', 'A', 'D'};
static const uint32_t version = 1;

template <typename T>
static void escribir(ofstream& f, const vector<T>& v) {
    f.write(reinterpret_cast<const char*>(v.data()), streamsize(v.size() * sizeof(T)));
}

template <typename T>
static void leer(ifstream& f, vector<T>& v, size_t n) {
    v.resize(n);
    f.read(reinterpret_cast<char*>(v.data()), streamsize(n * sizeof(T)));
}

GestorDatos::GestorDatos(Datos* df) : df(df) {}

void GestorDatos::menu() {
    avisar();
    int op = -1;
    while (op != 0) {
        cout << "\n-- MENU DATOS --"
                "\n1-Cargar fichero"
                "\n2-Generar"
                "\n3-Ajustar parametros"
                "\n4-Guardar fichero"
                "\n5-Ejemplo del enunciado"
                "\n6-Mostrar"
                "\n0-Volver"
                "\nIntroduce el numero de una opcion: ";
        if (!(cin >> op)) return;
        switch (op) {
            case 1: cout << (cargar(ruta()) ? "Cargado\n" : "No se pudo cargar\n"); break;
            case 2: generar(); cout << "Generado\n"; break;
            case 3: ajustar(); break;
            case 4: cout << (guardar(ruta()) ? "Guardado\n" : "No se pudo guardar\n"); break;
            case 5: ejemplo(); break;
            case 6: mostrar(); break;
        }
    }
}

void GestorDatos::preparar() {
    if (!cargar(ruta())) {
        generar();
        guardar(ruta());
    }
}

void GestorDatos::generar() {
    mt19937 gen(par.semilla);
    uint32_t kMax = min<uint32_t>(16, par.nVar);
    uniform_int_distribution<int> dCoef(1, 9);
    uniform_int_distribution<uint32_t> dK(0, kMax);
    uniform_int_distribution<uint32_t> dVar(0, par.nVar - 1);
    uniform_int_distribution<int> dExp(1, 5);
    uniform_real_distribution<double> dX(0.5, 1.5);

    df->nVar = par.nVar;
    df->nMon = par.nMon;
    df->nPun = par.nPun;
    df->coef.resize(par.nMon);
    df->ini.resize(size_t(par.nMon) + 1);
    df->var.clear();
    df->exp.clear();
    df->var.reserve(size_t(par.nMon) * kMax / 2);
    df->exp.reserve(size_t(par.nMon) * kMax / 2);

    df->ini[0] = 0;
    for (uint32_t m = 0; m < par.nMon; m++) {
        df->coef[m] = dCoef(gen);
        uint32_t k = dK(gen);
        for (uint32_t t = 0; t < k; t++) {
            uint32_t v;
            do v = dVar(gen);
            while (find(df->var.begin() + df->ini[m], df->var.end(), v) != df->var.end());
            df->var.push_back(v);
            df->exp.push_back(uint8_t(dExp(gen)));
        }
        df->ini[m + 1] = uint32_t(df->var.size());
    }

    df->x.resize(size_t(par.nPun) * par.nVar);
    for (double& v : df->x) v = dX(gen);
}

void GestorDatos::ejemplo() {
    par = {0, 3, 4, 1};
    df->nVar = 3;
    df->nMon = 4;
    df->nPun = 1;
    df->coef = {3, 4, 2, 5};
    df->ini = {0, 2, 4, 5, 5};
    df->var = {0, 1, 1, 2, 0};
    df->exp = {1, 1, 1, 1, 1};
    df->x = {1, 2, 3};
}

bool GestorDatos::guardar(const string& ruta) const {
    ofstream f(ruta, ios::binary);
    if (!f) return false;
    uint32_t cab[5] = {version, df->nVar, df->nMon, df->nPun, uint32_t(df->var.size())};
    f.write(<marca>, 4);
    f.write(reinterpret_cast<const char*>(cab), sizeof cab);
    escribir(f, df->coef);
    escribir(f, df->x);
    escribir(f, df->ini);
    escribir(f, df->var);
    escribir(f, df->exp);
    return f.good();
}

bool GestorDatos::cargar(const string& ruta) {
    ifstream f(ruta, ios::binary);
    if (!f) return false;
    char m[4];
    uint32_t cab[5];
    f.read(m, 4);
    f.read(reinterpret_cast<char*>(cab), sizeof cab);
    if (!f || !equal(m, m + 4, marca) || cab[0] != version) return false;
    df->nVar = cab[1];
    df->nMon = cab[2];
    df->nPun = cab[3];
    leer(f, df->coef, cab[2]);
    leer(f, df->x, size_t(cab[3]) * cab[1]);
    leer(f, df->ini, size_t(cab[2]) + 1);
    leer(f, df->var, cab[4]);
    leer(f, df->exp, cab[4]);
    return f.good();
}

void GestorDatos::mostrar() const {
    cout << "Variables: " << df->nVar
         << "\nMonomios:  " << df->nMon
         << "\nPuntos:    " << df->nPun
         << "\nTerminos:  " << df->var.size() << "\n";
    if (df->nMon > 10) return;
    for (uint32_t m = 0; m < df->nMon; m++) {
        if (m > 0) cout << " + ";
        cout << df->coef[m];
        for (uint32_t t = df->ini[m]; t < df->ini[m + 1]; t++)
            cout << "*x" << df->var[t] << "^" << int(df->exp[t]);
    }
    cout << "\n";
}

string GestorDatos::ruta() const {
    return "datos_" + to_string(par.semilla) + "_" + to_string(par.nVar) + "_" +
           to_string(par.nMon) + "_" + to_string(par.nPun) + ".bin";
}

void GestorDatos::avisar() const {
    cout << "\nNOTA: puedes generar los datos que quieras, pero tamanos muy grandes"
            "\npueden hacer que la ejecucion no termine en un tiempo razonable.\n";
}

void GestorDatos::ajustar() {
    cout << "Semilla (" << par.semilla << "): ";
    cin >> par.semilla;
    cout << "Variables (" << par.nVar << "): ";
    cin >> par.nVar;
    cout << "Monomios (" << par.nMon << "): ";
    cin >> par.nMon;
    cout << "Puntos (" << par.nPun << "): ";
    cin >> par.nPun;
    if (par.nVar == 0) par.nVar = 1;
    if (par.nPun == 0) par.nPun = 1;
}