#pragma once

// <<enumeration>> ResultadoLogin
// Posibles respuestas al intentar iniciar sesión
enum class ResultadoLogin
{
    EXITOSO,
    CREDENCIALES_INVALIDAS,
    CUENTA_DESACTIVADA,
    USUARIO_NO_EXISTE
};