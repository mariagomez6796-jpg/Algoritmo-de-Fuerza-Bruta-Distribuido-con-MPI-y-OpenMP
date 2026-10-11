#include <mpi.h>
#include <omp.h>

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <sstream>

#include "FuerzaBruta.h"

using namespace std;


// ======================================================
// TAGS MPI
// ======================================================

static const int TAG_RANGO_INICIO =
    10;

static const int TAG_RANGO_FIN =
    11;


static const int TAG_EXITO_TID =
    20;

static const int TAG_EXITO_IDX =
    21;

static const int TAG_EXITO_CLAVE =
    22;


static const int TAG_STOP =
    30;


// ======================================================
// INTEGRANTES
// ======================================================

void imprimirIntegrantes()
{
    cout
        << "\n"
        << "=============================================\n";

    cout
        << " PRACTICA 1.6 - FUERZA BRUTA MPI + OPENMP\n";

    cout
        << " EQUIPO 7\n";

    cout
        << "=============================================\n";


    cout
        << " Integrantes:\n";

    cout
        << " Gomez Enriquez Maria Fernanda\n";

    cout
        << " Lozano Perez Jose Manuel\n";

    cout
        << " Martinez Arias Eduardo Caleb\n";


    cout
        << "=============================================\n\n";
}


// ======================================================
// VALIDAR CLAVE
// ======================================================

bool claveValida(
    const char* clave
)
{
    if (
        clave == nullptr ||
        clave[0] == '\0'
    )
    {
        return false;
    }


    for (
        int i = 0;
        clave[i] != '\0';
        ++i
    )
    {
        char c =
            clave[i];


        bool letra =
            (
                c >= 'A' &&
                c <= 'Z'
            );


        bool numero =
            (
                c >= '0' &&
                c <= '9'
            );


        if (
            !letra &&
            !numero
        )
        {
            return false;
        }
    }


    return true;
}


// ======================================================
// MENU
// ======================================================

void mostrarMenu()
{
    cout
        << "\n"
        << "=============================================\n";

    cout
        << " PRACTICA 1.6 - EQUIPO 7\n";

    cout
        << "=============================================\n";

    cout
        << "1. Ingresar clave personalizada\n";

    cout
        << "2. Busqueda secuencial\n";

    cout
        << "3. Busqueda paralela distribuida MPI + OpenMP\n";

    cout
        << "4. Mostrar comparativa de tiempos\n";

    cout
        << "5. Salir\n";

    cout
        << "=============================================\n";

    cout
        << "Selecciona una opcion: ";

    cout.flush();
}


// ======================================================
// MAIN
// ======================================================

int main(
    int argc,
    char** argv
)
{
    // ==================================================
    // INICIAR MPI
    // ==================================================

    int provided =
        0;


    MPI_Init_thread(
        &argc,
        &argv,
        MPI_THREAD_FUNNELED,
        &provided
    );


    int rank =
        0;

    int size =
        0;


    MPI_Comm_rank(
        MPI_COMM_WORLD,
        &rank
    );


    MPI_Comm_size(
        MPI_COMM_WORLD,
        &size
    );


    // ==================================================
    // NOMBRE COMPUTADORA
    // ==================================================

    char processorName[
        MPI_MAX_PROCESSOR_NAME
    ];


    int nameLen =
        0;


    MPI_Get_processor_name(
        processorName,
        &nameLen
    );


    // ==================================================
    // OPENMP
    // ==================================================

    omp_set_dynamic(
        0
    );


    // ==================================================
    // LOG DEL NODO
    // ==================================================

    LogNodo log(
        rank,
        processorName
    );


    log.encabezado(
        size,
        omp_get_max_threads()
    );


    // ==================================================
    // DATOS INICIALES
    // ==================================================

    if (
        rank == 0
    )
    {
        imprimirIntegrantes();


        cout
            << "Maestro ejecutandose en: "
            << processorName
            << "\n";


        cout
            << "Procesos MPI detectados: "
            << size
            << "\n";


        cout
            << "Hilos OpenMP maximos por proceso: "
            << omp_get_max_threads()
            << "\n";
    }


    // ==================================================
    // VARIABLES
    // ==================================================

    char* clave =
        nullptr;


    int longitud =
        0;


    double tiempoSecuencial =
        -1.0;


    double tiempoParalelo =
        -1.0;


    bool salir =
        false;


    // ==================================================
    // MENU PRINCIPAL
    // ==================================================

    while (
        !salir
    )
    {
        int opcion =
            0;


        // ==============================================
        // SOLO MAESTRO LEE MENU
        // ==============================================

        if (
            rank == 0
        )
        {
            mostrarMenu();


            cin
                >> opcion;
        }


        MPI_Bcast(
            &opcion,
            1,
            MPI_INT,
            0,
            MPI_COMM_WORLD
        );


        // ==================================================
        // OPCION 1
        // ==================================================

        if (
            opcion == 1
        )
        {
            if (
                rank == 0
            )
            {
                char bufferTemporal[
                    128
                ];


                cout
                    << "\nIngresa la clave "
                    << "(solo A-Z y 0-9): ";


                cin
                    >> setw(128)
                    >> bufferTemporal;


                if (
                    !claveValida(
                        bufferTemporal
                    )
                )
                {
                    cout
                        << "\nERROR: "
                        << "la clave solo puede "
                        << "contener A-Z y 0-9.\n";


                    longitud =
                        0;
                }

                else
                {
                    delete[] clave;


                    longitud =
                        static_cast<int>(
                            strlen(
                                bufferTemporal
                            )
                        );


                    clave =
                        new char[
                            longitud + 1
                        ];


                    strcpy_s(
                        clave,
                        longitud + 1,
                        bufferTemporal
                    );


                    unsigned long long total =
                        0;


                    if (
                        !calcularTotalCombinaciones(
                            longitud,
                            total
                        )
                    )
                    {
                        cout
                            << "\nERROR: "
                            << "la longitud excede "
                            << "la capacidad permitida.\n";


                        delete[] clave;


                        clave =
                            nullptr;


                        longitud =
                            0;
                    }

                    else
                    {
                        cout
                            << "\nClave registrada correctamente.\n";


                        cout
                            << "Longitud: "
                            << longitud
                            << "\n";


                        cout
                            << "Espacio de busqueda: "
                            << "36^"
                            << longitud
                            << " = "
                            << total
                            << " combinaciones\n";
                    }
                }
            }


            // ==============================================
            // ENVIAR LONGITUD
            // ==============================================

            MPI_Bcast(
                &longitud,
                1,
                MPI_INT,
                0,
                MPI_COMM_WORLD
            );


            // ==============================================
            // MEMORIA EN ESCLAVOS
            // ==============================================

            if (
                rank != 0
            )
            {
                delete[] clave;


                clave =
                    nullptr;


                if (
                    longitud > 0
                )
                {
                    clave =
                        new char[
                            longitud + 1
                        ];
                }
            }


            // ==============================================
            // ENVIAR CLAVE
            // ==============================================

            if (
                longitud > 0
            )
            {
                MPI_Bcast(
                    clave,
                    longitud + 1,
                    MPI_CHAR,
                    0,
                    MPI_COMM_WORLD
                );


                // ==========================================
                // REGISTRAR CLAVE EN TODOS LOS LOGS
                // ==========================================

                ostringstream ss;


                ss
                    << "[CLAVE OBJETIVO] "
                    << "[Longitud: "
                    << longitud
                    << "] "
                    << "[Clave: "
                    << clave
                    << "]";


                log.escribir(
                    ss.str()
                );


                unsigned long long total =
                    0;


                calcularTotalCombinaciones(
                    longitud,
                    total
                );


                ostringstream espacio;


                espacio
                    << "[ESPACIO DE BUSQUEDA] "
                    << "36^"
                    << longitud
                    << " = "
                    << total
                    << " combinaciones";


                log.escribir(
                    espacio.str()
                );
            }


            MPI_Barrier(
                MPI_COMM_WORLD
            );
        }


        // ==================================================
        // OPCION 2
        // SECUENCIAL
        // ==================================================

        else if (
            opcion == 2
        )
        {
            if (
                longitud <= 0 ||
                clave == nullptr
            )
            {
                if (
                    rank == 0
                )
                {
                    cout
                        << "\nPrimero debes ingresar "
                        << "una clave con la opcion 1.\n";
                }


                continue;
            }


            if (
                rank == 0
            )
            {
                unsigned long long total =
                    0;


                calcularTotalCombinaciones(
                    longitud,
                    total
                );


                cout
                    << "\n"
                    << "=============================================\n"
                    << " BUSQUEDA SECUENCIAL\n"
                    << "=============================================\n";


                cout
                    << "Equipo: "
                    << processorName
                    << "\n";


                cout
                    << "Nodo MPI: 0\n";


                cout
                    << "Hilos usados: 1\n";


                cout
                    << "Total de combinaciones: "
                    << total
                    << "\n";


                log.escribir(
                    "============================================="
                );


                log.escribir(
                    "[MODO] BUSQUEDA SECUENCIAL"
                );


                double inicio =
                    MPI_Wtime();


                ResultadoBusqueda resultado =
                    busquedaSecuencial(
                        clave,
                        longitud,
                        0,
                        total - 1,
                        log,

                        // Detalle candidato por candidato
                        // solamente para <= 3 caracteres.
                        longitud <= 3
                    );


                tiempoSecuencial =
                    MPI_Wtime()
                    - inicio;


                if (
                    resultado.encontrado
                )
                {
                    char* encontrada =
                        new char[
                            longitud + 1
                        ];


                    convertirIndiceATexto(
                        resultado.indice,
                        encontrada,
                        longitud
                    );


                    cout
                        << "\n"
                        << "=============================================\n"
                        << "[EXITO] CLAVE ENCONTRADA\n"
                        << "=============================================\n";


                    cout
                        << "Clave: "
                        << encontrada
                        << "\n";


                    cout
                        << "Indice: "
                        << resultado.indice
                        << "\n";


                    cout
                        << fixed
                        << setprecision(6);


                    cout
                        << "Tiempo secuencial: "
                        << tiempoSecuencial
                        << " s\n";


                    cout
                        << "=============================================\n";


                    log.exito(
                        encontrada,
                        0,
                        resultado.indice,
                        tiempoSecuencial
                    );


                    delete[] encontrada;
                }

                else
                {
                    cout
                        << "\nClave no encontrada.\n";


                    log.escribir(
                        "[RESULTADO] Clave no encontrada."
                    );
                }
            }


            MPI_Barrier(
                MPI_COMM_WORLD
            );
        }


        // ==================================================
        // OPCION 3
        // MPI + OPENMP
        // ==================================================

        else if (
            opcion == 3
        )
        {
            if (
                longitud <= 0 ||
                clave == nullptr
            )
            {
                if (
                    rank == 0
                )
                {
                    cout
                        << "\nPrimero debes ingresar "
                        << "una clave con la opcion 1.\n";
                }


                continue;
            }


            if (
                size < 2
            )
            {
                if (
                    rank == 0
                )
                {
                    cout
                        << "\nERROR: "
                        << "ejecuta al menos "
                        << "2 procesos MPI.\n";
                }


                continue;
            }


            unsigned long long total =
                0;


            calcularTotalCombinaciones(
                longitud,
                total
            );


            // ==================================================
            // DETALLE COMPLETO SOLO PARA 3 CARACTERES
            // ==================================================

            bool detallado =
                (
                    longitud <= 3
                );


            log.escribir(
                "============================================="
            );


            log.escribir(
                "[MODO] BUSQUEDA PARALELA MPI + OPENMP"
            );


            MPI_Barrier(
                MPI_COMM_WORLD
            );


            double inicioGlobal =
                MPI_Wtime();


            // ==================================================
            // MAESTRO
            // ==================================================

            if (
                rank == 0
            )
            {
                int esclavos =
                    size - 1;


                unsigned long long base =
                    total
                    /
                    static_cast<
                        unsigned long long
                    >(
                        esclavos
                    );


                unsigned long long resto =
                    total
                    %
                    static_cast<
                        unsigned long long
                    >(
                        esclavos
                    );


                unsigned long long cursor =
                    0;


                cout
                    << "\n"
                    << "=============================================\n"
                    << " BUSQUEDA PARALELA DISTRIBUIDA\n"
                    << "=============================================\n";


                cout
                    << "Maestro: "
                    << processorName
                    << " | Nodo MPI: 0\n";


                cout
                    << "Espacio total: "
                    << total
                    << " combinaciones\n";


                cout
                    << "Esclavos MPI: "
                    << esclavos
                    << "\n\n";


                log.escribir(
                    "[MAESTRO] Inicio de distribucion de rangos."
                );


                // ==============================================
                // REPARTIR RANGOS
                // ==============================================

                for (
                    int r = 1;
                    r < size;
                    ++r
                )
                {
                    unsigned long long cantidad =
                        base
                        +
                        (
                            (
                                r <=
                                static_cast<int>(
                                    resto
                                )
                            )
                            ?
                            1ULL
                            :
                            0ULL
                        );


                    unsigned long long inicioRango =
                        cursor;


                    unsigned long long finRango =
                        cursor
                        +
                        cantidad
                        -
                        1ULL;


                    cursor +=
                        cantidad;


                    MPI_Send(
                        &inicioRango,
                        1,
                        MPI_UNSIGNED_LONG_LONG,
                        r,
                        TAG_RANGO_INICIO,
                        MPI_COMM_WORLD
                    );


                    MPI_Send(
                        &finRango,
                        1,
                        MPI_UNSIGNED_LONG_LONG,
                        r,
                        TAG_RANGO_FIN,
                        MPI_COMM_WORLD
                    );


                    cout
                        << "Nodo "
                        << r
                        << " -> rango ["
                        << inicioRango
                        << ", "
                        << finRango
                        << "]\n";


                    ostringstream ss;


                    ss
                        << "[DISTRIBUCION] "
                        << "[Nodo destino: "
                        << r
                        << "] "
                        << "[Rango: "
                        << inicioRango
                        << " - "
                        << finRango
                        << "]";


                    log.escribir(
                        ss.str()
                    );
                }


                // ==============================================
                // ESPERAR AL GANADOR
                // ==============================================

                int tidGanador =
                    -1;


                int rankGanador =
                    -1;


                unsigned long long indiceGanador =
                    0;


                char* claveEncontrada =
                    new char[
                        longitud + 1
                    ];


                claveEncontrada[0] =
                    '\0';


                MPI_Status status;


                MPI_Recv(
                    &tidGanador,
                    1,
                    MPI_INT,
                    MPI_ANY_SOURCE,
                    TAG_EXITO_TID,
                    MPI_COMM_WORLD,
                    &status
                );


                rankGanador =
                    status.MPI_SOURCE;


                MPI_Recv(
                    &indiceGanador,
                    1,
                    MPI_UNSIGNED_LONG_LONG,
                    rankGanador,
                    TAG_EXITO_IDX,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );


                MPI_Recv(
                    claveEncontrada,
                    longitud + 1,
                    MPI_CHAR,
                    rankGanador,
                    TAG_EXITO_CLAVE,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );


                // ==============================================
                // EARLY TERMINATION
                // ==============================================

                for (
                    int r = 1;
                    r < size;
                    ++r
                )
                {
                    if (
                        r !=
                        rankGanador
                    )
                    {
                        int stop =
                            1;


                        MPI_Send(
                            &stop,
                            1,
                            MPI_INT,
                            r,
                            TAG_STOP,
                            MPI_COMM_WORLD
                        );
                    }
                }


                tiempoParalelo =
                    MPI_Wtime()
                    -
                    inicioGlobal;


                // ==============================================
                // RESULTADO FINAL
                // ==============================================

                cout
                    << "\n"
                    << "=============================================\n"
                    << "[EXITO] CLAVE ENCONTRADA\n"
                    << "=============================================\n";


                cout
                    << "Clave: "
                    << claveEncontrada
                    << "\n";


                cout
                    << "Nodo MPI ganador: "
                    << rankGanador
                    << "\n";


                cout
                    << "Hilo OpenMP ganador: "
                    << tidGanador
                    << "\n";


                cout
                    << "Indice numerico: "
                    << indiceGanador
                    << "\n";


                cout
                    << fixed
                    << setprecision(6);


                cout
                    << "Tiempo paralelo distribuido: "
                    << tiempoParalelo
                    << " s\n";


                cout
                    << "=============================================\n";


                // ==============================================
                // LOG DEL MAESTRO
                // ==============================================

                {
                    ostringstream ss;


                    ss
                        << "[RESULTADO GLOBAL] "
                        << "[Clave: "
                        << claveEncontrada
                        << "] "
                        << "[Nodo ganador: "
                        << rankGanador
                        << "] "
                        << "[Hilo ganador: "
                        << tidGanador
                        << "] "
                        << "[Indice: "
                        << indiceGanador
                        << "]";


                    log.escribir(
                        ss.str()
                    );
                }


                {
                    ostringstream ss;


                    ss
                        << fixed
                        << setprecision(6);


                    ss
                        << "[TIEMPO MPI] "
                        << tiempoParalelo
                        << " segundos";


                    log.escribir(
                        ss.str()
                    );
                }


                log.escribir(
                    "[EARLY TERMINATION] "
                    "Se envio orden STOP a los nodos restantes."
                );


                delete[] claveEncontrada;
            }


            // ==================================================
            // ESCLAVOS
            // ==================================================

            else
            {
                unsigned long long inicioRango =
                    0;


                unsigned long long finRango =
                    0;


                MPI_Recv(
                    &inicioRango,
                    1,
                    MPI_UNSIGNED_LONG_LONG,
                    0,
                    TAG_RANGO_INICIO,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );


                MPI_Recv(
                    &finRango,
                    1,
                    MPI_UNSIGNED_LONG_LONG,
                    0,
                    TAG_RANGO_FIN,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );


                // ==============================================
                // CONSOLA LIMPIA
                // ==============================================

                cout
                    << "[Nodo MPI: "
                    << rank
                    << "] "
                    << "[Equipo: "
                    << processorName
                    << "] "
                    << "[Rango: "
                    << inicioRango
                    << " - "
                    << finRango
                    << "]"
                    << endl;


                // ==============================================
                // LOG
                // ==============================================

                log.rangoAsignado(
                    inicioRango,
                    finRango
                );


                {
                    ostringstream ss;


                    ss
                        << "[INICIO ESCLAVO] "
                        << "[Equipo: "
                        << processorName
                        << "] "
                        << "[Nodo MPI: "
                        << rank
                        << "] "
                        << "[Hilos OpenMP: "
                        << omp_get_max_threads()
                        << "]";


                    log.escribir(
                        ss.str()
                    );
                }


                // ==============================================
                // BUSCAR
                // ==============================================

                ResultadoBusqueda resultado =
                    busquedaParalelaOpenMP(
                        clave,
                        longitud,
                        inicioRango,
                        finRango,
                        rank,
                        processorName,
                        log,
                        detallado,
                        MPI_COMM_WORLD,
                        TAG_STOP
                    );


                // ==============================================
                // SI ESTE NODO GANO
                // ==============================================

                if (
                    resultado.encontrado
                )
                {
                    char* encontrada =
                        new char[
                            longitud + 1
                        ];


                    convertirIndiceATexto(
                        resultado.indice,
                        encontrada,
                        longitud
                    );


                    MPI_Send(
                        &resultado.hilo,
                        1,
                        MPI_INT,
                        0,
                        TAG_EXITO_TID,
                        MPI_COMM_WORLD
                    );


                    MPI_Send(
                        &resultado.indice,
                        1,
                        MPI_UNSIGNED_LONG_LONG,
                        0,
                        TAG_EXITO_IDX,
                        MPI_COMM_WORLD
                    );


                    MPI_Send(
                        encontrada,
                        longitud + 1,
                        MPI_CHAR,
                        0,
                        TAG_EXITO_CLAVE,
                        MPI_COMM_WORLD
                    );


                    log.exito(
                        encontrada,
                        resultado.hilo,
                        resultado.indice,
                        MPI_Wtime()
                        -
                        inicioGlobal
                    );


                    delete[] encontrada;
                }


                // ==============================================
                // SI COMPLETO SU RANGO ANTES DE RECIBIR STOP
                //
                // ESPERAR EL STOP PARA QUE TODOS LOS NODOS
                // FINALICEN CORRECTAMENTE.
                // ==============================================

                else if (
                    !resultado.detenido
                )
                {
                    int stop =
                        0;


                    MPI_Recv(
                        &stop,
                        1,
                        MPI_INT,
                        0,
                        TAG_STOP,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );


                    resultado.detenido =
                        true;


                    ostringstream ss;


                    ss
                        << "[STOP] "
                        << "[Equipo: "
                        << processorName
                        << "] "
                        << "[Nodo MPI: "
                        << rank
                        << "] "
                        << "Orden de detencion recibida "
                        << "despues de completar el rango.";


                    log.escribir(
                        ss.str()
                    );
                }


                // ==============================================
                // FINALIZACION DEL ESCLAVO
                // ==============================================

                {
                    ostringstream ss;


                    ss
                        << "[FIN] "
                        << "[Equipo: "
                        << processorName
                        << "] "
                        << "[Nodo MPI: "
                        << rank
                        << "] "
                        << "Proceso esclavo finalizado correctamente.";


                    log.escribir(
                        ss.str()
                    );
                }
            }


            // ==================================================
            // TODOS FINALIZARON
            // ==================================================

            MPI_Barrier(
                MPI_COMM_WORLD
            );


            cout.flush();

            fflush(
                stdout
            );


            if (
                rank == 0
            )
            {
                cout
                    << "\nBusqueda distribuida finalizada.\n";
            }
        }


        // ==================================================
        // OPCION 4
        // ==================================================

        else if (
            opcion == 4
        )
        {
            if (
                rank == 0
            )
            {
                cout
                    << "\n"
                    << "================ COMPARATIVA ================\n";


                if (
                    tiempoSecuencial > 0.0
                )
                {
                    cout
                        << fixed
                        << setprecision(6);


                    cout
                        << "Tiempo secuencial:             "
                        << tiempoSecuencial
                        << " s\n";
                }

                else
                {
                    cout
                        << "Tiempo secuencial:             "
                        << "SIN MEDIR\n";
                }


                if (
                    tiempoParalelo > 0.0
                )
                {
                    cout
                        << fixed
                        << setprecision(6);


                    cout
                        << "Tiempo paralelo distribuido:   "
                        << tiempoParalelo
                        << " s\n";
                }

                else
                {
                    cout
                        << "Tiempo paralelo distribuido:   "
                        << "SIN MEDIR\n";
                }


                if (
                    tiempoSecuencial > 0.0 &&
                    tiempoParalelo > 0.0
                )
                {
                    double speedup =
                        tiempoSecuencial
                        /
                        tiempoParalelo;


                    int p =
                        size - 1;


                    double eficiencia =
                        (
                            p > 0
                        )
                        ?
                        speedup
                        /
                        static_cast<double>(
                            p
                        )
                        :
                        0.0;


                    cout
                        << "Speedup S_p = T1/Tp:           "
                        << speedup
                        << "x\n";


                    cout
                        << "Procesos esclavos p:           "
                        << p
                        << "\n";


                    cout
                        << "Eficiencia E_p = S_p/p:        "
                        << eficiencia
                        << " ("
                        << eficiencia * 100.0
                        << "%)\n";


                    // ==========================================
                    // TAMBIEN GUARDAR COMPARATIVA EN LOG
                    // ==========================================

                    log.escribir(
                        "============================================="
                    );


                    log.escribir(
                        "[COMPARATIVA DE RENDIMIENTO]"
                    );


                    {
                        ostringstream ss;


                        ss
                            << fixed
                            << setprecision(6);


                        ss
                            << "Tiempo secuencial: "
                            << tiempoSecuencial
                            << " s";


                        log.escribir(
                            ss.str()
                        );
                    }


                    {
                        ostringstream ss;


                        ss
                            << fixed
                            << setprecision(6);


                        ss
                            << "Tiempo paralelo distribuido: "
                            << tiempoParalelo
                            << " s";


                        log.escribir(
                            ss.str()
                        );
                    }


                    {
                        ostringstream ss;


                        ss
                            << fixed
                            << setprecision(6);


                        ss
                            << "Speedup: "
                            << speedup
                            << "x";


                        log.escribir(
                            ss.str()
                        );
                    }


                    {
                        ostringstream ss;


                        ss
                            << fixed
                            << setprecision(6);


                        ss
                            << "Eficiencia: "
                            << eficiencia
                            << " ("
                            << eficiencia * 100.0
                            << "%)";


                        log.escribir(
                            ss.str()
                        );
                    }
                }


                cout
                    << "=============================================\n";
            }


            MPI_Barrier(
                MPI_COMM_WORLD
            );
        }


        // ==================================================
        // OPCION 5
        // ==================================================

        else if (
            opcion == 5
        )
        {
            log.escribir(
                "[SALIDA] Solicitud de salida recibida."
            );


            MPI_Barrier(
                MPI_COMM_WORLD
            );


            cout.flush();

            fflush(
                stdout
            );


            salir =
                true;
        }


        // ==================================================
        // OPCION INVALIDA
        // ==================================================

        else
        {
            if (
                rank == 0
            )
            {
                cout
                    << "\nOpcion invalida.\n";
            }
        }
    }


    // ======================================================
    // LIBERAR MEMORIA
    // ======================================================

    delete[] clave;


    clave =
        nullptr;


    // ======================================================
    // FINAL DEL LOG
    // ======================================================

    {
        ostringstream ss;


        ss
            << "[FIN PROGRAMA] "
            << "[Equipo: "
            << processorName
            << "] "
            << "[Nodo MPI: "
            << rank
            << "] "
            << "Proceso MPI finalizado correctamente.";


        log.escribir(
            ss.str()
        );
    }


    // ======================================================
    // SINCRONIZACION FINAL
    // ======================================================

    MPI_Barrier(
        MPI_COMM_WORLD
    );


    cout.flush();

    fflush(
        stdout
    );


    if (
        rank == 0
    )
    {
        cout
            << "\nTodos los procesos MPI "
            << "finalizaron correctamente.\n";
    }


    MPI_Barrier(
        MPI_COMM_WORLD
    );


    MPI_Finalize();


    return 0;
}