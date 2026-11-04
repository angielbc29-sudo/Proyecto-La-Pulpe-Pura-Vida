#pragma once

#include <memory>
#include "Usuario.h"

// Sesion: patrón Singleton
class Sesion
{
private:
    static Sesion* instancia;  
    std::shared_ptr<Usuario> usuarioActual; 

    Sesion();

    // Se bloquea la copia para que no se pueda duplicar la sesión
    Sesion(const Sesion&) = delete;
    Sesion& operator=(const Sesion&) = delete;

public:
    // Devuelve la única instancia o la crea si todavía no existe
    static Sesion& getInstancia();

    void     iniciar(std::shared_ptr<Usuario> u);
    void     cerrar();
    std::shared_ptr<Usuario> getUsuarioActual() const;
    bool     estaAutenticado() const;
};