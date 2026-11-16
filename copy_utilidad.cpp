#include "copy_utilidad.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>

bool comprobar_args(int argc, char* argv[]) {
    return argc == 3;
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

    int fd_destino = open(destino.c_str(), O_WRONLY | O_CREAT | O_TRUNC, dst_perms);
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

std::expected<void, std::system_error> copiar_fichero_fuera(const std::string& origen, 
                                                            const std::string& destino,
                                                            mode_t dst_perms) {
    std::string destino_final = nueva_ruta(origen, destino);
    auto resultado_copia = copiar_fichero(origen, destino_final, dst_perms);
    if (!resultado_copia) {
        return std::unexpected(resultado_copia.error());
    }
    return {};
}