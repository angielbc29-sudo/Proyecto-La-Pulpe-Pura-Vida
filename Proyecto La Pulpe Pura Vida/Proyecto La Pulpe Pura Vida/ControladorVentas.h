#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Producto.h"
#include "GestorInventario.h"
#include "Carrito.h"
#include "VentasCobro.h"

class Historial;   // [VENTAS] Se usa para registrar la venta al cobrar.

// [VENTAS] ControladorVentas: caso de uso "cobrar una venta".
// - Arma el carrito y valida el stock con GestorInventario.
// - Calcula subtotal, IVA y total delegando en Carrito.
// - Al cobrar en efectivo, descuenta inventario y registra la venta en Historial
//   como una sola operacion: si algo falla, revierte el stock y no guarda nada.
class ControladorVentas {
public:
    // [VENTAS] historial puede ser null, pero entonces no se permite cobrar
    // porque no habria donde registrar la venta.
    ControladorVentas(GestorInventario* inventario,
        std::shared_ptr<Historial> historial,
        const std::string& cajero);

    // [VENTAS] Manejo del carrito.
    bool agregarProducto(const std::string& codigo);
    void quitarUnidad(const std::string& codigo);
    void quitarProducto(const std::string& codigo);
    void vaciarCarrito();

    // [VENTAS] Totales del carrito. total() ya incluye IVA.
    double subtotal() const;
    double iva() const;
    double total() const;
    bool carritoVacio() const;

    // [VENTAS] Consulta del estado de la venta.
    const std::vector<Linea>& lineasCarrito() const;
    const VentasCobro& ventaCobrada() const;
    void fijarNumeroSiguiente(int numero);

    // [VENTAS] Cobra en efectivo e intenta registrar la venta en el historial.
    // Devuelve false y escribe el motivo en mensajeError si algo falla.
    bool cobrar(double efectivo, std::string* mensajeError = nullptr);

private:
    Carrito carrito_;
    GestorInventario* inventario_;
    std::shared_ptr<Historial> historial_;   // [VENTAS] Persistencia de la venta.
    std::string cajero_;
    VentasCobro ventaCobrada_;
    int numeroSiguiente_ = 1;

    // [VENTAS] Devuelve unidades al inventario si un cobro debe revertirse.
    void devolverStock(const Linea& linea);
};
