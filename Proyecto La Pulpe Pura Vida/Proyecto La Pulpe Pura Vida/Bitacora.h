#pragma once

#include <string>
#include <vector>
#include "RegistroBitacora.h"

// Bitacora: guarda en bitacora.json lo que hace cada usuario en el sistema.
// Cualquier módulo la usa con una sola línea, por ejemplo:
//     Bitacora::registrar("Ventas", "Venta cobrada", "Venta N.º 12, total 2260");
class Bitacora
{
public:
    // Agrega un renglón con la fecha y hora actuales y el usuario que tiene la sesión iniciada.
    // Si no se puede guardar, no hace nada (no debe detener lo que el usuario estaba haciendo).
    static void registrar(std::string modulo, std::string accion, std::string detalle = "");

    // Devuelve todos los renglones guardados, del más viejo al más nuevo
    static std::vector<RegistroBitacora> obtenerTodos();
};
