#ifndef COPY_UTILIDAD_HPP
#define COPY_UTILIDAD_HPP

#include <sys/types.h>

#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

#define TAMANO_BUFFER 131072

struct ParseArgsError {
    bool opcion_invalida = false;
    bool opcion_de_mas = false;
    bool hay_dos_ficheros = true;
};

enum class TipoCompresion {
    NADA,
    GZIP,
    BZIP2,
    XZ
};

struct Options {
    bool imprimir_ayuda = false;
    std::string origen = "";
    std::string destino = "";
    TipoCompresion modo_compresion = TipoCompresion::NADA;
};

std::expected<void, ParseArgsError> parse_args(int argc, char* argv[], Options& opts);

bool es_directorio(const std::string& destino);

std::string nueva_ruta(const std::string& origen, const std::string& destino);

bool es_mismo_fichero(const std::string& origen, const std::string& destino);

std::expected<void, std::system_error> copiar_fichero(const std::string& origen,
                                                      const std::string& destino,
                                                      mode_t dst_perms = 0644);

std::expected<void, std::system_error> copiar_fichero_comprimido(const std::string& origen,
                                                                 const std::string& destino,
                                                                 TipoCompresion modo_compresion,
                                                                 mode_t dst_perms = 0644);

void imprimir_uso(std::ostream& os = std::cout);

void imprimir_errores(const ParseArgsError& parse, std::ostream& os = std::cerr);

void imprimir_ayuda(std::ostream& os = std::cout);

std::string get_comando_compresion(TipoCompresion modo_compresion);

std::optional<std::string> find_command(const std::string& comando);

std::string get_extensiones(TipoCompresion compresion);

#endif // COPY_UTILIDAD_HPP