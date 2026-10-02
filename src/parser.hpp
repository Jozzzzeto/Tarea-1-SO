#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include "actividad.hpp"

//lee el archivo y crea las actividades
std::vector<Actividad> parsear_plan(
    const std::string& ruta
);

#endif