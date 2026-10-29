#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Usuario.h"

// <<interface>> IUsuarioRepository
// Define QUE se puede hacer con los usuarios guardados, sin decir COMO se guardan.
// Todos sus metodos son virtuales puros, por eso no necesita archivo .cpp
class IUsuarioRepository
{
public:
    virtual ~IUsuarioRepository() {}

    virtual std::vector<std::shared_ptr<Usuario>> obtenerTodos() = 0;
    virtual std::shared_ptr<Usuario> buscarPorUsername(std::string user) = 0;
    virtual bool guardar(std::shared_ptr<Usuario> u) = 0;
    virtual bool actualizar(std::shared_ptr<Usuario> u) = 0;
};
