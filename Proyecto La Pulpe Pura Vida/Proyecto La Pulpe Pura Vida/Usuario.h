#pragma once
#include <string>
#include "Permiso.h"

class Usuario
{
private:
    int         id;
    std::string username;
    std::string passwordHash;
    std::string nombreCompleto;
    std::string cedula;
    std::string telefono;
    bool        activo;

public:
    Usuario(int id, std::string user, std::string pass,
        std::string nombre, std::string ced, std::string tel, bool activo);

    virtual ~Usuario();

    int         getId() const;
    std::string getUsername() const;
    std::string getNombreCompleto() const;
    std::string getCedula() const;
    std::string getTelefono() const;
    bool        isActivo() const;

    void setActivo(bool estado);
    void setPasswordHash(std::string hash);

    virtual bool tienePermiso(Permiso p) const = 0;
    virtual std::string getNombreRol() const = 0;
    virtual std::string getDescripcionTurno() const = 0;
};