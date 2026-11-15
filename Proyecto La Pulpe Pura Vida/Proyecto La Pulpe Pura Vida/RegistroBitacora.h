#pragma once

#include <string>

// Un renglón de la bitácora: quién hizo qué, en cuál módulo y cuándo
struct RegistroBitacora
{
    std::string fechaHora;   // formato "yyyy-MM-dd HH:mm:ss"
    std::string usuario;
    std::string rol;
    std::string modulo;
    std::string accion;
    std::string detalle;
};
