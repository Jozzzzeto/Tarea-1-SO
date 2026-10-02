#include "parser.hpp"

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

//quita espacios al inicio y final
std::string trim(const std::string& texto) {

    size_t inicio = texto.find_first_not_of(" \t\r\n");

    if (inicio == std::string::npos) {
        return "";
    }

    size_t fin = texto.find_last_not_of(" \t\r\n");

    return texto.substr(
        inicio,
        fin - inicio + 1
    );
}


//separa una linea por :
std::vector<std::string> separar_campos(
    const std::string& linea
) {

    std::vector<std::string> campos;

    std::stringstream ss(linea);

    std::string campo;


    while (std::getline(ss, campo, ':')) {

        campos.push_back(
            trim(campo)
        );
    }


    return campos;
}


//separa dependencias por coma
std::vector<std::string> separar_dependencias(
    std::string campo
) {

    std::vector<std::string> dependencias;


    //quitar corchetes si existen
    campo.erase(
        std::remove(
            campo.begin(),
            campo.end(),
            '['
        ),
        campo.end()
    );


    campo.erase(
        std::remove(
            campo.begin(),
            campo.end(),
            ']'
        ),
        campo.end()
    );


    campo = trim(campo);


    if (campo.empty()) {
        return dependencias;
    }


    std::stringstream ss(campo);

    std::string dependencia;


    while (
        std::getline(
            ss,
            dependencia,
            ','
        )
    ) {

        dependencia = trim(dependencia);


        if (!dependencia.empty()) {

            dependencias.push_back(
                dependencia
            );
        }
    }


    return dependencias;
}


//genera tiempo entre 100 y 5000 ms
int tiempo_aleatorio() {

    static std::mt19937 generador(
        std::random_device{}()
    );


    static std::uniform_int_distribution<int> distribucion(
        100,
        5000
    );


    return distribucion(
        generador
    );
}


//lee el archivo y crea las actividades
std::vector<Actividad> parsear_plan(
    const std::string& ruta
) {

    std::ifstream archivo(ruta);


    if (!archivo.is_open()) {

        throw std::runtime_error(
            "No se pudo abrir el archivo: "
            + ruta
        );
    }


    std::vector<Actividad> actividades;


    //dependencias guardadas como ID antes de convertirlas
    std::vector<std::vector<std::string>> dependencias_ids;


    std::string linea;

    int numero_linea = 0;


    // =========================
    // LEER ARCHIVO
    // =========================

    while (
        std::getline(
            archivo,
            linea
        )
    ) {

        numero_linea++;


        linea = trim(linea);


        //ignorar lineas vacias
        if (linea.empty()) {
            continue;
        }


        std::vector<std::string> campos =
            separar_campos(linea);


        //id, nombre y tiempo como minimo
        if (campos.size() < 3) {

            throw std::runtime_error(
                "Formato invalido en linea "
                + std::to_string(numero_linea)
            );
        }


        std::string id =
            campos[0];


        std::string nombre =
            campos[1];


        std::string campo_tiempo =
            campos[2];


        std::string campo_dependencias = "";

        if (campos.size() >= 4) {

            campo_dependencias =
                campos[3];
        }


        //validar id
        if (id.empty()) {

            throw std::runtime_error(
                "ID vacio en linea "
                + std::to_string(numero_linea)
            );
        }


        //validar nombre
        if (nombre.empty()) {

            throw std::runtime_error(
                "Nombre vacio en linea "
                + std::to_string(numero_linea)
            );
        }


        int tiempo_ms;


        //tiempo aleatorio si viene vacio
        if (campo_tiempo.empty()) {

            tiempo_ms =
                tiempo_aleatorio();
        }

        else {

            try {

                tiempo_ms =
                    std::stoi(
                        campo_tiempo
                    );
            }

            catch (...) {

                throw std::runtime_error(
                    "Tiempo invalido en linea "
                    + std::to_string(numero_linea)
                );
            }


            if (tiempo_ms <= 0) {

                throw std::runtime_error(
                    "El tiempo debe ser mayor a 0 en linea "
                    + std::to_string(numero_linea)
                );
            }
        }


        //crear actividad
        Actividad actividad;

        actividad.id =
            id;

        actividad.nombre =
            nombre;

        actividad.tiempo_ms =
            tiempo_ms;


        actividades.push_back(
            actividad
        );


        //guardar dependencias como ID
        dependencias_ids.push_back(
            separar_dependencias(
                campo_dependencias
            )
        );
    }


    if (actividades.empty()) {

        throw std::runtime_error(
            "El archivo no contiene actividades"
        );
    }


    // =========================
    // MAPEAR ID -> INDICE
    // =========================

    std::unordered_map<std::string, int> indice_por_id;


    for (
        int i = 0;
        i < static_cast<int>(actividades.size());
        i++
    ) {

        const std::string& id =
            actividades[i].id;


        //no permitir IDs repetidos
        if (
            indice_por_id.count(id)
        ) {

            throw std::runtime_error(
                "ID repetido: "
                + id
            );
        }


        indice_por_id[id] =
            i;
    }


    // =========================
    // ARMAR DAG
    // =========================

    for (
        int i = 0;
        i < static_cast<int>(actividades.size());
        i++
    ) {

        for (
            const std::string& id_dependencia :
            dependencias_ids[i]
        ) {

            auto it =
                indice_por_id.find(
                    id_dependencia
                );


            //dependencia no existe
            if (
                it == indice_por_id.end()
            ) {

                throw std::runtime_error(
                    "La actividad "
                    + actividades[i].id
                    + " depende de un ID inexistente: "
                    + id_dependencia
                );
            }


            int indice_dependencia =
                it->second;


            //guardar dependencia
            actividades[i].dependencias.push_back(
                indice_dependencia
            );


            //guardar relacion inversa
            actividades[indice_dependencia]
                .dependientes
                .push_back(i);
        }


        //cantidad de dependencias pendientes
        actividades[i].deps_pendientes =
            static_cast<int>(
                actividades[i].dependencias.size()
            );


        //si no tiene dependencias ya puede ejecutarse
        if (
            actividades[i].deps_pendientes == 0
        ) {

            actividades[i].estado =
                Estado::LISTA;
        }

        else {

            actividades[i].estado =
                Estado::PENDIENTE;
        }
    }


    return actividades;
}