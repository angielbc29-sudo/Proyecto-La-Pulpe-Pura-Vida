#pragma once

#include <string>
#include <vector>
#include "Producto.h"
#include "GestorInventario.h"
#include "Carrito.h"
#include "VentasCobro.h"
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

    const std::vector<Linea>& lineasCarrito() const;
    const VentasCobro& ventaCobrada() const;
    void fijarNumeroSiguiente(int numero);

    bool cobrar(double efectivo, std::string* mensajeError = nullptr);

private:
    Carrito carrito_;
    GestorInventario* inventario_;
    std::string cajero_;
    VentasCobro ventaCobrada_;
    int numeroSiguiente_ = 1;
};