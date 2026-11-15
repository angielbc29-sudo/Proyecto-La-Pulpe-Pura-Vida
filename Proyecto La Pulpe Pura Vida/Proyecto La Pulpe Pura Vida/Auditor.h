#pragma once

#include <string>
#include "Usuario.h"

// Auditor: usuario que solo puede consultar la bitácora del sistema
class Auditor : public Usuario
{
public:
    Auditor(int id, std::string user, std::string pass, std::string nombre,
        std::string ced, std::string tel, bool activo);

    bool tienePermiso(Permiso p) const override;
    std::string getNombreRol() const override;
    std::string getDescripcionTurno() const override;
};
