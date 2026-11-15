#include "Auditor.h"

Auditor::Auditor(int id, std::string user, std::string pass, std::string nombre,
    std::string ced, std::string tel, bool activo)
    : Usuario(id, user, pass, nombre, ced, tel, activo)
{
}

bool Auditor::tienePermiso(Permiso p) const
{
    // El auditor solo puede ver la bitácora
    return p == Permiso::VER_BITACORA;
}

std::string Auditor::getNombreRol() const
{
    return "Auditor";
}

std::string Auditor::getDescripcionTurno() const
{
    return "Sin turno fijo";
}
