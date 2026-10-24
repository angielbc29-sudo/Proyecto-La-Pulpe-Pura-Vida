#pragma once

#include <string>
#include "Usuario.h"

// Administrador hereda de Usuario
class Administrador : public Usuario
{
public:
    // Constructor
    Administrador(int id, std::string user, std::string pass, std::string nombre,
        std::string ced, std::string tel, bool activo);

    // Implementación de los métodos abstractos de Usuario
    bool        tienePermiso(Permiso p) const override;
    std::string getNombreRol() const override;
    std::string getDescripcionTurno() const override;
};