#include "planificador.hpp"
#include <vector>

int main() {
    std::vector<Actividad> actividades;

    Actividad a;
    a.id = "1";
    a.nombre = "Actividad_1";
    a.tiempo_ms = 2000;
    a.estado = Estado::LISTA;
    a.dependientes = {2};

    Actividad b;
    b.id = "2";
    b.nombre = "Actividad_2";
    b.tiempo_ms = 2000;
    b.estado = Estado::LISTA;
    b.dependientes = {2};

    Actividad c;
    c.id = "3";
    c.nombre = "Actividad_3";
    c.tiempo_ms = 2000;
    c.estado = Estado::PENDIENTE;
    c.dependencias = {0, 1};
    c.deps_pendientes = 2;

    actividades = {a, b, c};

    ejecutar_planificador(actividades, 2);

    return 0;
}