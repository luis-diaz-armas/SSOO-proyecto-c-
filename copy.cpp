#include "copy_utilidad.h"

#include <iostream>
#include <stdexcept>
#include <string>

#define EXIT_EXITO 0
#define EXIT_FALLO 1

int main_protegido(int argc, char* argv[]) {
    Options opts;
    auto parse = parse_args(argc, argv, opts);
    if (opts.imprimir_ayuda) {
        imprimir_ayuda();
        return EXIT_EXITO;
    }
    if (!parse) {
        imprimir_errores(parse.error());
        return EXIT_FALLO;
    }
    if (es_mismo_fichero(opts.origen, opts.destino)) {
        std::cout << "\tcopy: son el mismo fichero\n";
        return EXIT_EXITO;
    }
    if (opts.modo_compresion == TipoCompresion::NADA) {
        copiar_fichero(opts.origen, opts.destino);
    } else {
        copiar_fichero_comprimido(opts.origen, opts.destino, opts.modo_compresion);
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