#include "copy_utilidad.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <expected>
#include <string>
#include <string_view>
#include <vector>

std::expected<void, ParseArgsError> parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    ParseArgsError errores;

    for (std::string_view arg : args) {
        if (arg == "-h" || arg == "--help") {
            opts.imprimir_ayuda = true;
            return {};
        }

        if (arg == "-z" || arg == "-j" || arg == "-x") {
            if (opts.modo_compresion != TipoCompresion::NADA) {
                errores.opcion_de_mas = true;
                return std::unexpected(errores);
            }

            if (arg == "-z") opts.modo_compresion = TipoCompresion::GZIP;
            else if (arg == "-j") opts.modo_compresion = TipoCompresion::BZIP2;
            else opts.modo_compresion = TipoCompresion::XZ;
            continue;
        }

        if (arg.starts_with('-')) {
            errores.opcion_invalida = true;
            return std::unexpected(errores);
        }

        if (opts.origen.empty()) {
            opts.origen = std::string(arg);
        } else if (opts.destino.empty()) {
            opts.destino = std::string(arg);
        } else {
            errores.opcion_de_mas = true;
            return std::unexpected(errores);
        }
    }

    if (opts.destino.empty()) {
        errores.hay_dos_ficheros = false;
        return std::unexpected(errores);
    }

    return {};
}

bool es_directorio(const std::string& destino) {
    struct stat st;
    if (stat(destino.c_str(), &st) == -1) {
        if (errno == ENOENT) {
            return false;
        }
        throw std::runtime_error("No se pudo acceder a " + destino + ": " + std::strerror(errno));
    }
    return S_ISDIR(st.st_mode);
}

std::string nueva_ruta(const std::string& origen, const std::string& destino) {
    std::filesystem::path ruta_origen = origen;
    std::filesystem::path ruta_destino_dir = destino;
    std::filesystem::path ruta_final = ruta_destino_dir / ruta_origen.filename();
    return std::filesystem::absolute(ruta_final).string();
}

bool es_mismo_fichero(const std::string& origen, const std::string& destino) {
    struct stat st1;
    if (stat(origen.c_str(), &st1) == -1) {
        throw std::runtime_error("No se pudo acceder a " + origen + ": " + std::strerror(errno));
    }
    struct stat st2;
    if (stat(destino.c_str(), &st2) == -1) {
        if (errno == ENOENT) {
            return false;
        }
        throw std::runtime_error("No se pudo acceder a " + destino + ": " + std::strerror(errno));
    }
    return (st1.st_dev == st2.st_dev) && (st1.st_ino == st2.st_ino);
}

std::expected<void, std::system_error> copiar_fichero(const std::string& origen, 
                                                      const std::string& destino,
                                                      mode_t dst_perms) {
    struct Closer {
        int fd;
        ~Closer() { if (fd >= 0) close(fd); }
    };

    int fd_origen = open(origen.c_str(), O_RDONLY);
    if (fd_origen < 0) {
        return std::unexpected(std::system_error(errno, std::system_category(), 
                               std::string("Error al abrir " +  origen)));
    }
    Closer guard_origen(fd_origen);

    std::string destino_def;
    if (es_directorio(destino)) {
        destino_def = nueva_ruta(origen, destino);
    } else {
        destino_def = destino;
    }

    int fd_destino = open(destino_def.c_str(), O_WRONLY | O_CREAT | O_TRUNC, dst_perms);
    if (fd_destino < 0) {
        return std::unexpected(std::system_error(errno, std::system_category(), 
                               std::string("error al abrir " +  destino)));
    }
    Closer guard_destino(fd_destino);

    char buffer[TAMANO_BUFFER];
    ssize_t bytes_leidos = 0;
    while ((bytes_leidos = read(fd_origen, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_escritos = 0;
        while (bytes_escritos < bytes_leidos) {
            ssize_t n = write(fd_destino, buffer + bytes_escritos, bytes_leidos - bytes_escritos);
            if (n < 0) {
                if (errno == EINTR) continue;
                return std::unexpected(std::system_error(errno, std::system_category(), 
                                       "error al escribir en " + destino));
            }
            bytes_escritos += n;
        }
    }
    
    if (bytes_leidos < 0) {
        return std::unexpected(std::system_error(errno, std::system_category(), 
                                                 "Error al leer de " + origen));
    }
    return {};
}

void imprimir_uso(std::ostream& os) {
    os << "uso: copy [-z | -j | -x] ORIGEN DESTINO\n";

}

std::expected<void, std::system_error> copiar_fichero_comprimido(const std::string& origen, 
                                                                 const std::string& destino,
                                                                 TipoCompresion modo_compresion,
                                                                 mode_t dst_perms) {
    std::optional<std::string> ruta_comando = find_command(modo_compresion);
    if (!ruta_comando.has_value()) {
        return std::unexpected(std::system_error(errno, std::system_category(), 
                               "El comando de compresión de la opción seleccionada no está instalado\n"));
    }

    std::string destino_def;
    if (es_directorio(destino)) {
        destino_def = nueva_ruta(origen, destino);
    } else {
        destino_def = destino;
    }
    destino_def.append(get_extensiones(modo_compresion));

    int pipefd[2];

    int tuberia = pipe(pipefd);
    if (tuberia < 0) {
        return std::unexpected(std::system_error(errno, std::system_category(), 
                               "error al abrir la tubería\n"));
    }

}

void imprimir_errores(const ParseArgsError& parse, std::ostream& os) {
    if (parse.opcion_invalida) {
        os << "copy: error: opción desconocida\n";
    }
    if (parse.opcion_de_mas) {
        os << "copy: error: solo se admite una opción de compresión\n";
    }
    if (!parse.hay_dos_ficheros) {
        os << "copy: error: no se han pasado las direcciones de ORIGEN y/o DESTINO\n";
    }
}

void imprimir_ayuda(std::ostream& os) {
    os << "ayuda\n";
}

std::string get_comando_compresion(TipoCompresion modo_compresion) {
    if (modo_compresion == TipoCompresion::GZIP) {
        return "gzip";
    } else if (modo_compresion == TipoCompresion::BZIP2) {
        return "bzip2";
    } else {
        return "xz";
    }
}

std::optional<std::string> find_command(const std::string& comando) {
    if (comando.empty()) return std::nullopt;

    const char* env_p = std::getenv("PATH");
    if (env_p == nullptr) return std::nullopt;

    std::string path = env_p;
    std::string ruta;
    std::vector<std::string> rutas;

    for (char c : path) {
        if (c != ':') {
            ruta.push_back(c);
        } else {
            rutas.push_back(ruta + "/" + comando);
            ruta = "";
        }
    }

    rutas.push_back(ruta + "/" + comando);

    for (const std::string& candidato : rutas) {
        if (access(candidato.c_str(), X_OK) == 0) {
            return candidato;
        }
    }

    return std::nullopt;
}

std::string get_extensiones(TipoCompresion compresion) {
    if (compresion == TipoCompresion::NADA) {
        return "";
    } else if (compresion == TipoCompresion::GZIP) {
        return ".gz";
    } else if (compresion == TipoCompresion::BZIP2) {
        return ".bz2";
    } else {
        return ".xz";
    }
}