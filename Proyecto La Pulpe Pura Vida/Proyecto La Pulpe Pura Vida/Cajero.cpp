#pragma once

#include <memory>
#include "Usuario.h"

// Sesion: patrón Singleton (solo existe una sesión en toda la aplicación)
class Sesion
{
private:
    static Sesion* instancia;   // única instancia de la clase (atributo estático)
    std::shared_ptr<Usuario> usuarioActual;   // puntero inteligente: se libera solo

    // Constructor privado: nadie fuera de la clase puede hacer "Sesion s;"
    Sesion();

    // Se bloquea la copia para que no se pueda duplicar la sesión
    Sesion(const Sesion&) = delete;
    Sesion& operator=(const Sesion&) = delete;

public:
    // Devuelve la única instancia (la crea si todavía no existe)
    static Sesion& getInstancia();

    void     iniciar(std::shared_ptr<Usuario> u);
    void     cerrar();
    std::shared_ptr<Usuario> getUsuarioActual() const;
    bool     estaAutenticado() const;
};