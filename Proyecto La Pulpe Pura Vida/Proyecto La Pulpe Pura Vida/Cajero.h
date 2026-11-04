#pragma once
#include <string>
#include "Usuario.h"
#include "Turno.h"

class Cajero : public Usuario
{
private:
    Turno turno;

public:
    Cajero(int id, std::string user, std::string pass, std::string nombre,
        std::string ced, std::string tel, bool activo, Turno t);

    Turno getTurno() const;
    void  setTurno(Turno t);

    bool tienePermiso(Permiso p) const override;
    std::string getNombreRol() const override;
    std::string getDescripcionTurno() const override;
};