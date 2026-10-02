#include "planificador.hpp"
#include "parser.hpp"
#include <vector>
#include <iostream>


int main(int argc, char* argv[]) {
//asegura que al ejecutar reciba los argumentos necesarios
    if (argc != 3) {
        std::cerr << "Uso: ./planificador plan.txt K || ./planificador src/plan.txt K \n";
        return 1;
    }

    std::string archivo = argv[1];
    
    //declara la variable para K procesos
    int K;

    try {
        K = std::stoi(argv[2]); //transforma K en valor entero
    }
    catch (...) {
        std::cerr << "K debe ser un numero entero\n";
        return 1;
    }

    if (K <= 0) {
        std::cerr << "K debe ser mayor a 0\n";
        return 1;
    }

    //transforma el archivo en el DAG
    try {
        std::vector<Actividad> actividades =
            parsear_plan(archivo);

        ejecutar_planificador(
            actividades,
            K
        );

        std::cout << "\n--- Resultado final ---\n";

        for (const auto& act : actividades) {
            std::cout
                << act.nombre
                << " -> "
                << estado_a_texto(act.estado)
                << "\n";
        }

    }
    //avisa en caso de error
    catch (const std::exception& e) {

        std::cerr
            << "Error: "
            << e.what()
            << "\n";

        return 1;
    }

    return 0;
}