#pragma once

#include <string>
#include <vector>
#include "Producto.h"

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

