#pragma once

#include <memory>
#include <string>
#include "IUsuarioRepository.h"
#include "ResultadoLogin.h"
#include "Usuario.h"

// Valida usuario, contraseña y estado de la cuenta
class ServicioAutentication
{
private:
    IUsuarioRepository* repo;   // de dónde se leen los usuarios (JSON u otro)

    // Cifra la contraseña escrita con SHA-256 y la compara con la guardada
    bool verificarHash(std::string passPlano, std::string hashGuardado);

public:
    ServicioAutentication(IUsuarioRepository* repo);

    // Devuelve el resultado del intento de inicio de sesión.
    // Si es EXITOSO, deja el usuario en usuarioOut.
    ResultadoLogin autenticar(std::string user, std::string passPlano,
        std::shared_ptr<Usuario>& usuarioOut);
};