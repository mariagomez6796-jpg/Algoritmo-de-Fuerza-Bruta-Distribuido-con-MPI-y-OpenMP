#pragma once

#include <mpi.h>
#include <fstream>
#include <string>


// ======================================================
// RESULTADO DE BUSQUEDA
// ======================================================

struct ResultadoBusqueda
{
    bool encontrado;
    bool detenido;

    unsigned long long indice;

    int hilo;


    ResultadoBusqueda()
    {
        encontrado = false;
        detenido = false;
        indice = 0;
        hilo = -1;
    }
};


// ======================================================
// CLASE PARA LOGS
// ======================================================

class LogNodo
{
private:

    std::ofstream archivo;

    int rank;

    std::string equipo;


public:

    LogNodo(
        int rankMPI,
        const char* nombreEquipo
    );


    ~LogNodo();


    void encabezado(
        int procesosMPI,
        int hilosMaximos
    );


    void escribir(
        const std::string& mensaje
    );


    void rangoAsignado(
        unsigned long long inicio,
        unsigned long long fin
    );


    void candidato(
        const char* candidato,
        int hilo
    );


    void exito(
        const char* clave,
        int hilo,
        unsigned long long indice,
        double tiempo
    );
};


// ======================================================
// FUNCIONES GENERALES
// ======================================================

bool calcularTotalCombinaciones(
    int longitud,
    unsigned long long& total
);


void convertirIndiceATexto(
    unsigned long long indice,
    char* salida,
    int longitud
);


// ======================================================
// BUSQUEDA SECUENCIAL
// ======================================================

ResultadoBusqueda busquedaSecuencial(
    const char* objetivo,
    int longitud,
    unsigned long long inicio,
    unsigned long long fin,
    LogNodo& log,
    bool detallado
);


// ======================================================
// BUSQUEDA MPI + OPENMP
// ======================================================

ResultadoBusqueda busquedaParalelaOpenMP(
    const char* objetivo,
    int longitud,
    unsigned long long inicio,
    unsigned long long fin,
    int rank,
    const char* equipo,
    LogNodo& log,
    bool detallado,
    MPI_Comm comm,
    int tagStop
);