#include "Administrador.h"

// ---------------- Constructor ----------------

Administrador::Administrador(int id, std::string user, std::string pass, std::string nombre,
    std::string ced, std::string tel, bool activo)
    : Usuario(id, user, pass, nombre, ced, tel, activo)   // llama al constructor de la clase padre
{
}

// ---------------- Métodos heredados ----------------

bool Administrador::tienePermiso(Permiso p) const
{
    // El administrador tiene todos los permisos
    return true;
}

std::string Administrador::getNombreRol() const
{
    return "Administrador";
}

std::string Administrador::getDescripcionTurno() const
{
    // El administrador no tiene un turno asignado
    return "Sin turno fijo";
}