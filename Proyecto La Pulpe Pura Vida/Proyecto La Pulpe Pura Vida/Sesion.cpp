#include "Sesion.h"

// El atributo estático se define (y se inicializa) una sola vez, fuera de la clase
Sesion* Sesion::instancia = nullptr;

// ---------------- Constructor privado ----------------

Sesion::Sesion()
{
    this->usuarioActual = nullptr;   // al inicio no hay nadie con sesión iniciada
}

// ---------------- Singleton ----------------

Sesion& Sesion::getInstancia()
{
    if (instancia == nullptr)
    {
        instancia = new Sesion();
    }
    return *instancia;
}

// ---------------- Manejo de la sesión ----------------

void Sesion::iniciar(std::shared_ptr<Usuario> u)
{
    usuarioActual = u;
}

void Sesion::cerrar()
{
    // Solo se "suelta" el puntero; el usuario sigue existiendo mientras alguien más lo use (por ejemplo el repositorio)
    usuarioActual = nullptr;
}

std::shared_ptr<Usuario> Sesion::getUsuarioActual() const
{
    return usuarioActual;
}

bool Sesion::estaAutenticado() const
{
    return usuarioActual != nullptr;
}