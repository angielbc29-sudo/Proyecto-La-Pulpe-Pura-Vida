#include "Usuario.h"

// ---------------- Constructor / Destructor ----------------

Usuario::Usuario(int id, std::string user, std::string pass,
    std::string nombre, std::string ced, std::string tel, bool activo)
{
    this->id = id;
    this->username = user;
    this->passwordHash = pass;
    this->nombreCompleto = nombre;
    this->cedula = ced;
    this->telefono = tel;
    this->activo = activo;
}

Usuario::~Usuario()
{
}

// ---------------- Getters ----------------

int Usuario::getId() const
{
    return id;
}

std::string Usuario::getUsername() const
{
    return username;
}

std::string Usuario::getNombreCompleto() const
{
    return nombreCompleto;
}

std::string Usuario::getCedula() const
{
    return cedula;
}

std::string Usuario::getTelefono() const
{
    return telefono;
}

bool Usuario::isActivo() const
{
    return activo;
}

// ---------------- Setters ----------------

void Usuario::setActivo(bool estado)
{
    activo = estado;
}

void Usuario::setPasswordHash(std::string hash)
{
    passwordHash = hash;
}