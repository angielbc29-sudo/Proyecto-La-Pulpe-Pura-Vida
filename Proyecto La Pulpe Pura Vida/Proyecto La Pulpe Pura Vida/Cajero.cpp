#include "Cajero.h"

// ---------------- Constructor ----------------

Cajero::Cajero(int id, std::string user, std::string pass, std::string nombre,
    std::string ced, std::string tel, bool activo, Turno t)
    : Usuario(id, user, pass, nombre, ced, tel, activo)   // llama al constructor de la clase padre
{
    this->turno = t;
}

// ---------------- Turno ----------------

Turno Cajero::getTurno() const
{
    return turno;
}

void Cajero::setTurno(Turno t)
{
    turno = t;
}

// ---------------- Métodos heredados ----------------

bool Cajero::tienePermiso(Permiso p) const
{
    // El cajero solo puede realizar ventas y consultar el historial
    return p == Permiso::REALIZAR_VENTA
        || p == Permiso::CONSULTAR_HISTORIAL;
}

std::string Cajero::getNombreRol() const
{
    return "Cajero";
}

std::string Cajero::getDescripcionTurno() const
{
    switch (turno)
    {
    case Turno::MANANA:   return "Turno de la manana";
    case Turno::TARDE:    return "Turno de la tarde";
    case Turno::NOCHE:    return "Turno de la noche";
    case Turno::COMPLETO: return "Turno completo";
    }
    return "Turno desconocido";
}