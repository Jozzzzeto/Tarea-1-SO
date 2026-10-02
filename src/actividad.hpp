#ifndef ACTIVIDAD_HPP
#define ACTIVIDAD_HPP

#include <string>
#include <vector>
#include <sys/types.h>

//estados posibles de una actividad
enum class Estado {
    PENDIENTE,
    LISTA,
    CORRIENDO,
    HECHA,
    FALLIDA,
    ABORTADA
};

//datos de cada actividad
struct Actividad {
    std::string id;
    std::string nombre;
    int tiempo_ms;

    //actividades de las que depende
    std::vector<int> dependencias;

    //actividades que dependen de esta
    std::vector<int> dependientes;

    //cantidad de dependencias que faltan
    int deps_pendientes = 0;

    //estado actual
    Estado estado = Estado::PENDIENTE;

    //pid del proceso
    pid_t pid = -1;

    //mensaje generado al terminar
    std::string mensaje;
};

//convierte el estado de la actividad a texto
const char* estado_a_texto(Estado estado); 

#endif