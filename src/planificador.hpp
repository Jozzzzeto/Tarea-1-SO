#ifndef PLANIFICADOR_HPP
#define PLANIFICADOR_HPP

#include <vector>
#include "actividad.hpp"

constexpr int TAM_MSG = 256;

//ejecuta las actividades respetando el limite K
void ejecutar_planificador(
    std::vector<Actividad>& actividades,
    int K
);

#endif