#ifndef COPY_UTILIDAD_HPP
#define COPY_UTILIDAD_HPP

#include <expected>
#include <string>
#include <system_error>

#define TAMANO_BUFFER 131072

bool comprobar_args(int argc, char* argv[]);

bool es_directorio(const std::string& destino);

std::string nueva_ruta(const std::string& origen, const std::string& destino);

bool es_mismo_fichero(const std::string& origen, const std::string& destino);

std::expected<void, std::system_error> copiar_fichero(const std::string& origen, 
                                                      const std::string& destino,
                                                      mode_t dst_perms = 0644);

std::expected<void, std::system_error> copiar_fichero_fuera(const std::string& origen, 
                                                            const std::string& destino,
                                                            mode_t dst_perms = 0644);

#endif // COPY_UTILIDAD_HPP