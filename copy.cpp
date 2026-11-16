#include "copy_utilidad.h"

#include <iostream>
#include <stdexcept>
#include <string>

#define EXIT_EXITO 0
#define EXIT_FALLO 1

int main_protegido(int argc, char* argv[]) {
    if (!comprobar_args(argc, argv)) {
        std::cerr << "\tcopy: argumentos invalidados\n";
        return EXIT_FALLO;
    }
    if (!es_directorio(std::string(argv[2]))) {
        if (es_mismo_fichero(std::string(argv[1]), std::string(argv[2]))) {
            std::cout << "\tcopy: son el mismo fichero\n";
            return EXIT_EXITO;
        }
        copiar_fichero(std::string(argv[1]), std::string(argv[2]));
    } else {
        copiar_fichero_fuera(std::string(argv[1]), std::string(argv[2]));
    }
    return EXIT_EXITO;
}

int main(int argc, char* argv[]) {
    try {
        return main_protegido(argc, argv);
    } catch (const std::system_error& e) {
        std::cerr << "error inseperado (" << e.code().value()
                  << "): " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error insperado: " << e.what() << "\n";
    }
    return EXIT_FALLO;
}