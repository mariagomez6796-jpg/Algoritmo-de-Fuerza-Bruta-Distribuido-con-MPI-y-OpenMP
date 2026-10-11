#include "FuerzaBruta.h"

#include <omp.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <limits>
#include <atomic>

using namespace std;


// ======================================================
// ALFABETO
// ======================================================

static const char* ALFABETO =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static const unsigned long long BASE =
    36ULL;


// ======================================================
// LOG
// ======================================================

LogNodo::LogNodo(
    int rankMPI,
    const char* nombreEquipo
)
    : rank(rankMPI),
      equipo(nombreEquipo)
{
    ostringstream nombreArchivo;

    nombreArchivo
        << "log_equipo_"
        << equipo
        << "_nodo_"
        << rank
        << ".txt";


    archivo.open(
        nombreArchivo.str(),
        ios::out | ios::trunc
    );
}


LogNodo::~LogNodo()
{
    if (archivo.is_open())
    {
        archivo.flush();
        archivo.close();
    }
}


void LogNodo::encabezado(
    int procesosMPI,
    int hilosMaximos
)
{
    if (!archivo.is_open())
        return;


    archivo
        << "=====================================================\n";

    archivo
        << " PRACTICA 1.6 - FUERZA BRUTA MPI + OPENMP\n";

    archivo
        << " EQUIPO 7\n";

    archivo
        << "=====================================================\n";


    archivo
        << "Equipo/Host: "
        << equipo
        << "\n";


    archivo
        << "Nodo MPI: "
        << rank
        << "\n";


    archivo
        << "Procesos MPI totales: "
        << procesosMPI
        << "\n";


    archivo
        << "Hilos OpenMP maximos: "
        << hilosMaximos
        << "\n";


    archivo
        << "Estado: PROCESO MPI INICIADO\n";


    archivo
        << "=====================================================\n";


    archivo.flush();
}


void LogNodo::escribir(
    const string& mensaje
)
{
#pragma omp critical(LOG_ARCHIVO)
    {
        if (archivo.is_open())
        {
            archivo
                << mensaje
                << endl;

            archivo.flush();
        }
    }
}


void LogNodo::rangoAsignado(
    unsigned long long inicio,
    unsigned long long fin
)
{
    ostringstream ss;


    ss
        << "[RANGO ASIGNADO] "
        << "[Equipo: "
        << equipo
        << "] "
        << "[Nodo MPI: "
        << rank
        << "] "
        << "[Inicio: "
        << inicio
        << "] "
        << "[Fin: "
        << fin
        << "]";


    escribir(
        ss.str()
    );
}


void LogNodo::candidato(
    const char* candidato,
    int hilo
)
{
#pragma omp critical(LOG_ARCHIVO)
    {
        if (archivo.is_open())
        {
            archivo
                << "[Equipo: "
                << equipo
                << "] "

                << "[Nodo MPI: "
                << rank
                << "] "

                << "[Hilo OpenMP: "
                << hilo
                << "] "

                << "[Candidato: "
                << candidato
                << "] "

                << "[Estado: Evaluando]"
                << endl;


            archivo.flush();
        }
    }
}


void LogNodo::exito(
    const char* clave,
    int hilo,
    unsigned long long indice,
    double tiempo
)
{
#pragma omp critical(LOG_ARCHIVO)
    {
        if (archivo.is_open())
        {
            archivo
                << fixed
                << setprecision(6);


            archivo
                << "[EXITO] "

                << "[Equipo: "
                << equipo
                << "] "

                << "[Nodo MPI: "
                << rank
                << "] "

                << "[Hilo OpenMP: "
                << hilo
                << "] "

                << "[Clave Encontrada: \""
                << clave
                << "\"] "

                << "[Indice: "
                << indice
                << "] "

                << "[Tiempo Parcial: "
                << tiempo
                << " s]"
                << endl;


            archivo.flush();
        }
    }
}


// ======================================================
// CALCULAR 36^N
// ======================================================

bool calcularTotalCombinaciones(
    int longitud,
    unsigned long long& total
)
{
    if (longitud <= 0)
        return false;


    total =
        1ULL;


    for (
        int i = 0;
        i < longitud;
        ++i
    )
    {
        if (
            total >
            numeric_limits<unsigned long long>::max()
            / BASE
        )
        {
            return false;
        }


        total *=
            BASE;
    }


    return true;
}


// ======================================================
// INDICE -> TEXTO BASE 36
// ======================================================

void convertirIndiceATexto(
    unsigned long long indice,
    char* salida,
    int longitud
)
{
    for (
        int posicion = longitud - 1;
        posicion >= 0;
        --posicion
    )
    {
        salida[posicion] =
            ALFABETO[
                indice % BASE
            ];


        indice /=
            BASE;
    }


    salida[longitud] =
        '\0';
}


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
)
{
    ResultadoBusqueda resultado;


    char* candidato =
        new char[
            longitud + 1
        ];


    log.escribir(
        "[INICIO] Busqueda secuencial iniciada."
    );


    for (
        unsigned long long indice = inicio;
        ;
        ++indice
    )
    {
        convertirIndiceATexto(
            indice,
            candidato,
            longitud
        );


        // ==================================================
        // PARA 3 CARACTERES:
        // GUARDAR TODO EN LOG
        // NO MOSTRAR CANDIDATO EN CONSOLA
        // ==================================================

        if (detallado)
        {
            log.candidato(
                candidato,
                0
            );
        }


        // ==================================================
        // COMPARAR
        // ==================================================

        if (
            strcmp(
                candidato,
                objetivo
            ) == 0
        )
        {
            resultado.encontrado =
                true;

            resultado.indice =
                indice;

            resultado.hilo =
                0;


            break;
        }


        if (
            indice ==
            fin
        )
        {
            break;
        }
    }


    if (
        resultado.encontrado
    )
    {
        log.escribir(
            "[FIN] Busqueda secuencial finalizada con EXITO."
        );
    }

    else
    {
        log.escribir(
            "[FIN] Busqueda secuencial finalizada sin coincidencia."
        );
    }


    delete[] candidato;


    return resultado;
}


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
)
{
    ResultadoBusqueda resultado;


    atomic<bool> encontrado(
        false
    );

    atomic<bool> detener(
        false
    );


    unsigned long long indiceGanador =
        0;


    int hiloGanador =
        -1;


    // ==================================================
    // BLOQUES GRANDES:
    // menos sobrecarga OpenMP que 4096
    // ==================================================

    const unsigned long long TAM_BLOQUE =
        1000000ULL;


    unsigned long long actual =
        inicio;


    // ==================================================
    // INFORMACION DEL LOG
    // ==================================================

    {
        ostringstream ss;


        ss
            << "[INICIO BUSQUEDA] "
            << "[Equipo: "
            << equipo
            << "] "
            << "[Nodo MPI: "
            << rank
            << "] "
            << "[Hilos OpenMP disponibles: "
            << omp_get_max_threads()
            << "]";


        log.escribir(
            ss.str()
        );
    }


    // ==================================================
    // BUCLE PRINCIPAL
    // ==================================================

    while (
        actual <= fin &&
        !encontrado.load() &&
        !detener.load()
    )
    {
        // ==================================================
        // COMPROBAR SI EL MAESTRO ENVIO STOP
        // ==================================================

        int flagStop =
            0;


        MPI_Status status;


        MPI_Iprobe(
            0,
            tagStop,
            comm,
            &flagStop,
            &status
        );


        if (flagStop)
        {
            int stop =
                0;


            MPI_Recv(
                &stop,
                1,
                MPI_INT,
                0,
                tagStop,
                comm,
                MPI_STATUS_IGNORE
            );


            detener.store(
                true
            );


            resultado.detenido =
                true;


            // SOLO LOG
            // NO MOSTRAR EN CONSOLA

            ostringstream ss;


            ss
                << "[STOP] "
                << "[Equipo: "
                << equipo
                << "] "
                << "[Nodo MPI: "
                << rank
                << "] "
                << "Orden de detencion recibida.";


            log.escribir(
                ss.str()
            );


            break;
        }


        // ==================================================
        // CALCULAR BLOQUE
        // ==================================================

        unsigned long long bloqueFin =
            actual
            + TAM_BLOQUE
            - 1ULL;


        if (
            bloqueFin < actual ||
            bloqueFin > fin
        )
        {
            bloqueFin =
                fin;
        }


        // ==================================================
        // OPENMP
        // ==================================================

#pragma omp parallel
        {
            int tid =
                omp_get_thread_num();


            char* candidato =
                new char[
                    longitud + 1
                ];


            // Para fuerza bruta todos los candidatos
            // tienen practicamente el mismo costo.
            // static reduce la sobrecarga.

#pragma omp for schedule(static)
            for (
                long long offset = 0;
                offset <=
                static_cast<long long>(
                    bloqueFin - actual
                );
                ++offset
            )
            {
                if (
                    encontrado.load() ||
                    detener.load()
                )
                {
                    continue;
                }


                unsigned long long indice =
                    actual
                    +
                    static_cast<
                        unsigned long long
                    >(offset);


                convertirIndiceATexto(
                    indice,
                    candidato,
                    longitud
                );


                // ==========================================
                // CLAVE DE 3 CARACTERES
                //
                // CANDIDATO POR CANDIDATO SOLO EN LOG
                // ==========================================

                if (detallado)
                {
                    log.candidato(
                        candidato,
                        tid
                    );
                }


                // ==========================================
                // COMPARAR
                // ==========================================

                if (
                    strcmp(
                        candidato,
                        objetivo
                    ) == 0
                )
                {
                    bool esperado =
                        false;


                    if (
                        encontrado.compare_exchange_strong(
                            esperado,
                            true
                        )
                    )
                    {
#pragma omp critical(HALLAZGO)
                        {
                            indiceGanador =
                                indice;


                            hiloGanador =
                                tid;
                        }
                    }
                }
            }


            delete[] candidato;
        }


        // ==================================================
        // ESTE NODO ENCONTRO LA CLAVE
        // ==================================================

        if (
            encontrado.load()
        )
        {
            break;
        }


        // ==================================================
        // SIGUIENTE BLOQUE
        // ==================================================

        if (
            bloqueFin ==
            fin
        )
        {
            break;
        }


        actual =
            bloqueFin
            + 1ULL;
    }


    // ======================================================
    // RESULTADO
    // ======================================================

    resultado.encontrado =
        encontrado.load();


    resultado.indice =
        indiceGanador;


    resultado.hilo =
        hiloGanador;


    // ======================================================
    // MOSTRAR SOLO EXITO EN CONSOLA
    // ======================================================

    if (
        resultado.encontrado
    )
    {
        char* claveEncontrada =
            new char[
                longitud + 1
            ];


        convertirIndiceATexto(
            resultado.indice,
            claveEncontrada,
            longitud
        );


#pragma omp critical(SALIDA_EXITO)
        {
            cout
                << "\n"
                << "---------------------------------------------"
                << endl;


            cout
                << "[EXITO] "
                << "[Equipo: "
                << equipo
                << "] "
                << "[Nodo MPI: "
                << rank
                << "] "
                << "[Hilo OpenMP: "
                << resultado.hilo
                << "] "
                << "[Clave: "
                << claveEncontrada
                << "]"
                << endl;


            cout
                << "---------------------------------------------"
                << endl;
        }


        delete[] claveEncontrada;
    }


    // ======================================================
    // LOG FINAL DEL NODO
    // ======================================================

    if (
        resultado.encontrado
    )
    {
        log.escribir(
            "[FIN BUSQUEDA] Nodo finalizo despues de encontrar la clave."
        );
    }

    else if (
        resultado.detenido
    )
    {
        log.escribir(
            "[FIN BUSQUEDA] Nodo detenido por Early Termination."
        );
    }

    else
    {
        log.escribir(
            "[FIN BUSQUEDA] Nodo completo su rango sin encontrar la clave."
        );
    }


    return resultado;
}