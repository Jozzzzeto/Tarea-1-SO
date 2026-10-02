#include "planificador.hpp"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>

namespace {

//indica si se presiono ctrl+c
volatile sig_atomic_t g_sigint_recibida = 0;


//se ejecuta cuando llega SIGINT
void manejador_sigint(int /*signum*/) {
    g_sigint_recibida = 1;
}


//configura el manejo de ctrl+c
void instalar_manejador_sigint() {
    struct sigaction sa{};

    sa.sa_handler = manejador_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        throw std::runtime_error(
            "No se pudo instalar el manejador de SIGINT"
        );
    }
}


//info de un hijo que esta corriendo
struct ProcesoActivo {
    pid_t pid;

    //actividad que corresponde al proceso
    int idx_actividad;

    //extremo de lectura del padre
    int pipe_out_read;
};


//junta los mensajes de las dependencias
std::string combinar_mensajes_dependencias(
    const std::vector<Actividad>& acts,
    int idx
) {
    std::string combinado;

    for (int dep_idx : acts[idx].dependencias) {
        combinado += acts[dep_idx].mensaje;
        combinado += "; ";
    }

    return combinado;
}


//crea el proceso que ejecuta una actividad
ProcesoActivo lanzar_actividad(
    std::vector<Actividad>& acts,
    int idx
) {

    //padre -> hijo
    int pipe_in[2];

    //hijo -> padre
    int pipe_out[2];


    //crear pipe de entrada
    if (pipe(pipe_in) == -1) {
        throw std::runtime_error(
            "No se pudo crear pipe_in para la actividad "
            + acts[idx].id
        );
    }


    //crear pipe de salida
    if (pipe(pipe_out) == -1) {

        close(pipe_in[0]);
        close(pipe_in[1]);

        throw std::runtime_error(
            "No se pudo crear pipe_out para la actividad "
            + acts[idx].id
        );
    }


    //mensajes que vienen de las dependencias
    std::string mensaje_entrada =
        combinar_mensajes_dependencias(acts, idx);


    //crear hijo
    pid_t pid = fork();


    //error al crear hijo
    if (pid < 0) {

        close(pipe_in[0]);
        close(pipe_in[1]);

        close(pipe_out[0]);
        close(pipe_out[1]);

        throw std::runtime_error(
            "fork() fallo para la actividad "
            + acts[idx].id
        );
    }


    // =========================
    // HIJO
    // =========================
    if (pid == 0) {

        //el hijo no escribe en pipe_in
        close(pipe_in[1]);

        //el hijo no lee pipe_out
        close(pipe_out[0]);


        char buf[TAM_MSG] = {0};


        //recibe mensaje del padre
        ssize_t bytes_leidos =
            read(
                pipe_in[0],
                buf,
                TAM_MSG - 1
            );


        if (bytes_leidos < 0) {

            close(pipe_in[0]);
            close(pipe_out[1]);

            _exit(1);
        }


        close(pipe_in[0]);


        //simular tiempo de la actividad
        usleep(
            static_cast<useconds_t>(
                acts[idx].tiempo_ms
            ) * 1000
        );


        //mensaje que genera la actividad
        std::string msg_salida =
            acts[idx].nombre
            + " completado. Insumo: "
            + buf;


        size_t cantidad =
            std::min(
                msg_salida.size(),
                static_cast<size_t>(TAM_MSG - 1)
            );


        //mandar resultado al padre
        ssize_t bytes_escritos =
            write(
                pipe_out[1],
                msg_salida.c_str(),
                cantidad
            );


        if (bytes_escritos < 0) {

            close(pipe_out[1]);

            _exit(1);
        }


        close(pipe_out[1]);


        //termino bien
        _exit(0);
    }


    // =========================
    // PADRE
    // =========================

    //el padre no lee pipe_in
    close(pipe_in[0]);

    //el padre no escribe pipe_out
    close(pipe_out[1]);


    size_t cantidad =
        std::min(
            mensaje_entrada.size(),
            static_cast<size_t>(TAM_MSG - 1)
        );


    //mandar dependencias al hijo
    ssize_t bytes_escritos =
        write(
            pipe_in[1],
            mensaje_entrada.c_str(),
            cantidad
        );


    if (bytes_escritos < 0) {

        close(pipe_in[1]);
        close(pipe_out[0]);

        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);

        throw std::runtime_error(
            "Error enviando mensaje a la actividad "
            + acts[idx].id
        );
    }


    close(pipe_in[1]);


    //marcar actividad como ejecutandose
    acts[idx].estado = Estado::CORRIENDO;
    acts[idx].pid = pid;

        std::cout << "INICIA: " << acts[idx].nombre
          << " PID=" << pid << std::endl;

    //guardar el hijo activo
    return ProcesoActivo{
        pid,
        idx,
        pipe_out[0]
    };
}


//aborta todas las actividades que dependen de una que fallo
void abortar_rama(
    std::vector<Actividad>& acts,
    int idx_raiz,
    int& restantes
) {

    std::queue<int> cola;

    cola.push(idx_raiz);


    while (!cola.empty()) {

        int idx = cola.front();
        cola.pop();


        //revisar las actividades que dependen de esta
        for (int dep_idx : acts[idx].dependientes) {

            if (
                acts[dep_idx].estado == Estado::PENDIENTE ||
                acts[dep_idx].estado == Estado::LISTA
            ) {

                acts[dep_idx].estado = Estado::ABORTADA;

                restantes--;

                cola.push(dep_idx);
            }
        }
    }
}


//aborta todo cuando se presiona ctrl+c
void abortar_todo(
    std::vector<Actividad>& acts,
    std::vector<ProcesoActivo>& activos
) {

    std::cerr
        << "\nSIGINT recibido: abortando todas las actividades...\n";


    //matar hijos activos
    for (const auto& proceso : activos) {

        kill(
            proceso.pid,
            SIGTERM
        );
    }


    //esperar hijos y cerrar pipes
    for (const auto& proceso : activos) {

        waitpid(
            proceso.pid,
            nullptr,
            0
        );

        close(
            proceso.pipe_out_read
        );

        acts[
            proceso.idx_actividad
        ].estado = Estado::ABORTADA;
    }


    activos.clear();


    //abortar las que aun no comenzaban
    for (auto& act : acts) {

        if (
            act.estado == Estado::PENDIENTE ||
            act.estado == Estado::LISTA
        ) {

            act.estado = Estado::ABORTADA;
        }
    }
}

} //namespace



void ejecutar_planificador(
    std::vector<Actividad>& acts,
    int K
) {

    //activar manejo de ctrl+c
    instalar_manejador_sigint();


    //cola de actividades listas
    std::queue<int> listos;


    //agregar actividades sin dependencias pendientes
    for (
        int i = 0;
        i < static_cast<int>(acts.size());
        i++
    ) {

        if (acts[i].estado == Estado::LISTA) {
            listos.push(i);
        }
    }


    //procesos que estan corriendo
    std::vector<ProcesoActivo> activos;


    //actividades que faltan
    int restantes =
        static_cast<int>(acts.size());


    while (restantes > 0) {


        //ctrl+c
        if (g_sigint_recibida) {

            abortar_todo(
                acts,
                activos
            );

            return;
        }


        //crear hijos mientras haya espacio en K
        while (
            activos.size() < static_cast<size_t>(K)
            &&
            !listos.empty()
        ) {

            int idx = listos.front();

            listos.pop();


            ProcesoActivo nuevo =
                lanzar_actividad(
                    acts,
                    idx
                );


            activos.push_back(nuevo);
        }


        //si quedan actividades pero ninguna puede avanzar
        if (activos.empty()) {

            std::cerr
                << "Advertencia: quedan actividades sin poder avanzar "
                << "(posible ciclo en el plan).\n";


            for (auto& act : acts) {

                if (
                    act.estado == Estado::PENDIENTE ||
                    act.estado == Estado::LISTA
                ) {

                    act.estado = Estado::ABORTADA;
                }
            }


            return;
        }


        //esperar que termine cualquier hijo
        int status = 0;


        pid_t terminado =
            waitpid(
                -1,
                &status,
                0
            );


        //waitpid fue interrumpido
        if (terminado == -1) {

            if (errno == EINTR) {
                continue;
            }


            throw std::runtime_error(
                "waitpid() fallo inesperadamente"
            );
        }


        //buscar el hijo que termino
        auto it =
            std::find_if(
                activos.begin(),
                activos.end(),

                [&](const ProcesoActivo& proceso) {
                    return proceso.pid == terminado;
                }
            );


        if (it == activos.end()) {
            continue;
        }


        int idx =
            it->idx_actividad;


        //extremo de lectura del padre
        int pipe_read =
            it->pipe_out_read;


        //sacar de activos
        activos.erase(it);


        //revisar si el hijo termino bien
        bool ok =
            WIFEXITED(status)
            &&
            WEXITSTATUS(status) == 0;


        // =========================
        // ACTIVIDAD TERMINO BIEN
        // =========================
        if (ok) {

            char buf[TAM_MSG] = {0};


            //leer mensaje enviado por el hijo
            ssize_t bytes_leidos =
                read(
                    pipe_read,
                    buf,
                    TAM_MSG - 1
                );


            if (bytes_leidos < 0) {

                close(pipe_read);

                throw std::runtime_error(
                    "Error leyendo resultado de actividad "
                    + acts[idx].id
                );
            }


            //guardar mensaje
            acts[idx].mensaje = buf;

            std::cout << "MENSAJE RECIBIDO: " << acts[idx].mensaje << std::endl;
            //marcar como terminada
            acts[idx].estado = Estado::HECHA;
std::       cout << "TERMINA: " << acts[idx].nombre << std::endl;

            restantes--;


            //actualizar las actividades que dependen de esta
            for (
                int dep_idx :
                acts[idx].dependientes
            ) {

                acts[dep_idx].deps_pendientes--;


                //si ya no le falta ninguna dependencia
                if (
                    acts[dep_idx].deps_pendientes == 0
                    &&
                    acts[dep_idx].estado == Estado::PENDIENTE
                ) {

                    acts[dep_idx].estado = Estado::LISTA;

                    listos.push(dep_idx);
                }
            }
        }


        // =========================
        // ACTIVIDAD FALLO
        // =========================
        else {

            acts[idx].estado = Estado::FALLIDA;

            restantes--;


            //abortar solo su rama
            abortar_rama(
                acts,
                idx,
                restantes
            );
        }


        close(pipe_read);
    }
}