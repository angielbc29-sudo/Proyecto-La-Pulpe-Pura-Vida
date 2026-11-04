#include "Administrador.h"

Administrador::Administrador(int id, std::string user, std::string pass, std::string nombre,
    std::string ced, std::string tel, bool activo)
    : Usuario(id, user, pass, nombre, ced, tel, activo)   
{
}

bool Administrador::tienePermiso(Permiso p) const
{
    return true;
}

std::string Administrador::getNombreRol() const
{
    return "Administrador";
}

std::string Administrador::getDescripcionTurno() const
{
    return "Sin turno fijo";
}