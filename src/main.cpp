#include "planificador.hpp"
#include <vector>

int main() {
    std::vector<Actividad> actividades;

    Actividad a;
    a.id = "1";
    a.nombre = "Actividad_1";
    a.tiempo_ms = 3000;
    a.estado = Estado::LISTA;

    Actividad b;
    b.id = "2";
    b.nombre = "Actividad_2";
    b.tiempo_ms = 3000;
    b.estado = Estado::LISTA;

    Actividad c;
    c.id = "3";
    c.nombre = "Actividad_3";
    c.tiempo_ms = 3000;
    c.estado = Estado::LISTA;

    Actividad d;
    d.id = "4";
    d.nombre = "Actividad_4";
    d.tiempo_ms = 3000;
    d.estado = Estado::LISTA;

    actividades = {a, b, c, d};

    ejecutar_planificador(actividades, 2);

    return 0;
}