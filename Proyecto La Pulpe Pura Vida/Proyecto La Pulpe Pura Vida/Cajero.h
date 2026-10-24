#pragma once

#include <string>
#include "Usuario.h"
#include "Turno.h"

// Cajero hereda de Usuario
class Cajero : public Usuario
{
private:
    Turno turno;

public:
    // Constructor
    Cajero(int id, std::string user, std::string pass, std::string nombre,
        std::string ced, std::string tel, bool activo, Turno t);

    // Getter y setter del turno
    Turno getTurno() const;
    void  setTurno(Turno t);

    // Implementación de los métodos abstractos de Usuario
    bool        tienePermiso(Permiso p) const override;
    std::string getNombreRol() const override;
    std::string getDescripcionTurno() const override;
};