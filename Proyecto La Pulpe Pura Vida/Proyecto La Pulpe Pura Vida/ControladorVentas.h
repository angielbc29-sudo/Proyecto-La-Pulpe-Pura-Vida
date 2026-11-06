#pragma once

#include <string>
#include <vector>
#include "Producto.h"
#include "GestorInventario.h"
// ControladorVentas: Gestiona la interacción entre el carrito de compras y el inventario.
// Permite agregar y quitar productos del carrito, calcular totales y procesar el cobro.
class ControladorVentas {
public:
    ControladorVentas(GestorInventario* inventario, const std::string& cajero);

    bool agregarProducto(const std::string& codigo);
    void quitarUnidad(const std::string& codigo);
    void quitarProducto(const std::string& codigo);
    void vaciarCarrito();

    double subtotal() const;
    double iva() const;
    double total() const;
    bool carritoVacio() const;

    bool cobrar(double efectivo, std::string* mensajeError = nullptr);

private:
    Carrito carrito_;
    GestorInventario* inventario_;
    std::string cajero_;
};