#pragma once

#include <string>
#include "Usuario.h"


class Administrador : public Usuario
{
public:
    Administrador(int id, std::string user, std::string pass, std::string nombre,
        std::string ced, std::string tel, bool activo);

    bool tienePermiso(Permiso p) const override;
    std::string getNombreRol() const override;
    std::string getDescripcionTurno() const override;
};