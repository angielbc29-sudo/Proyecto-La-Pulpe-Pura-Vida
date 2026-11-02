#pragma once

#include <string>
#include <vector>
#include "Producto.h"
#include "GestorInventario.h"

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

//Carrito: Representa el carrito de compras, que contiene una lista de líneas de venta.
// Proporciona métodos para agregar, quitar y vaciar productos, así como para calcular el subtotal, IVA y total de la compra.
class Carrito {
public:
    bool agregar(const Producto& producto);
    bool quitarUnidad(const std::string& codigo);
    bool quitar(const std::string& codigo);
    void vaciar();
    bool estaVacio() const;
    int cantidadArticulos() const;
    double subtotal() const;
    double iva() const;
    double total() const;
    const std::vector<LineaVenta>& lineas() const;

private:
    int buscarIndice(const std::string& codigo) const;

    std::vector<LineaVenta> lineas_;
};

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