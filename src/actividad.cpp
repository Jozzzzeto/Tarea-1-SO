#include "actividad.hpp"

const char* estado_a_texto(Estado estado) {

    switch (estado) {

        case Estado::PENDIENTE:
            return "PENDIENTE";

        case Estado::LISTA:
            return "LISTA";

        case Estado::CORRIENDO:
            return "CORRIENDO";

        case Estado::HECHA:
            return "HECHA";

        case Estado::FALLIDA:
            return "FALLIDA";

        case Estado::ABORTADA:
            return "ABORTADA";
    }

    return "DESCONOCIDO";
}