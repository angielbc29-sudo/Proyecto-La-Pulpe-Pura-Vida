#pragma once

#include <string>
#include <vector>
#include "Producto.h"

//Linea de venta: Representa un producto agregado al carrito de compras, con su código, nombre, precio unitario y cantidad.
//Subtotal: Calcula el subtotal de la línea de venta multiplicando el precio unitario por la cantidad.

struct LineaVenta {
    std::string codigo;
    std::string nombre;
    double precioUnitario = 0.0;
    int cantidad = 0;

    double subtotal() const {
        return precioUnitario * cantidad;
    }
};