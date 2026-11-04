#pragma once

#include <memory>
#include <string>
#include <vector>
#include "IUsuarioRepository.h"

// Guarda y lee los usuarios en un archivo JSON.
// Cumple el "contrato" de IUsuarioRepository (relación de Realización del diagrama).
class UsuarioJsonRepository : public IUsuarioRepository
{
private:
    std::string rutaArchivo;

    // Convierten entre la lista de usuarios y el contenido del archivo JSON
    bool serializar(std::vector<std::shared_ptr<Usuario>> lista);
    std::vector<std::shared_ptr<Usuario>> deserializar();

public:
    // Si el archivo no existe, lo crea con los usuarios de prueba
    UsuarioJsonRepository(std::string ruta);

    std::vector<std::shared_ptr<Usuario>> obtenerTodos() override;
    std::shared_ptr<Usuario> buscarPorUsername(std::string user) override;
    bool guardar(std::shared_ptr<Usuario> u) override;
    bool actualizar(std::shared_ptr<Usuario> u) override;
};