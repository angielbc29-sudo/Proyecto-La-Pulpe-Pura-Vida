#include "ControladorVentas.h"

#include <QDateTime>

ControladorVentas::ControladorVentas(GestorInventario* inventario, const std::string& cajero)
    : inventario_(inventario), cajero_(cajero)
{
}

bool ControladorVentas::agregarProducto(const std::string& codigo) {
    if (inventario_ == nullptr)
        return false;

    Producto* producto = inventario_->buscarPorCodigo(codigo);
    if (producto == nullptr || !producto->getActivo())
        return false;

    return carrito_.agregar(*producto);
}

void ControladorVentas::quitarUnidad(const std::string& codigo) {
    carrito_.quitarUnidad(codigo);
}

void ControladorVentas::quitarProducto(const std::string& codigo) {
    carrito_.quitar(codigo);
}

void ControladorVentas::vaciarCarrito() {
    carrito_.vaciar();
}

double ControladorVentas::subtotal() const {
    return carrito_.subtotal();
}

double ControladorVentas::iva() const {
    return carrito_.iva();
}

double ControladorVentas::total() const {
    return carrito_.total();
}

bool ControladorVentas::carritoVacio() const {
    return carrito_.estaVacio();
}

const std::vector<Linea>& ControladorVentas::lineasCarrito() const {
    return carrito_.lineas();
}

const VentasCobro& ControladorVentas::ventaCobrada() const {
    return ventaCobrada_;
}

void ControladorVentas::fijarNumeroSiguiente(int numero) {
    numeroSiguiente_ = numero > 0 ? numero : 1;
}

bool ControladorVentas::cobrar(double efectivo, std::string* mensajeError) {
    if (carrito_.estaVacio()) {
        if (mensajeError) *mensajeError = "El carrito está vacío.";
        return false;
    }

    const double totalVenta = carrito_.total();
    if (efectivo < totalVenta) {
        if (mensajeError) *mensajeError = "El efectivo no cubre el total de la venta.";
        return false;
    }

    for (const Linea& linea : carrito_.lineas()) {
        if (!inventario_->verificarDisponibilidad(linea.codigo().toStdString(), linea.cantidad())) {
            if (mensajeError) *mensajeError = "Stock insuficiente de " + linea.nombre().toStdString() + ".";
            return false;
        }
    }

    for (const Linea& linea : carrito_.lineas()) {
        if (!inventario_->descontarStock(linea.codigo().toStdString(), linea.cantidad())) {
            if (mensajeError) *mensajeError = "No se pudo descontar el stock de " + linea.nombre().toStdString() + ".";
            return false;
        }
    }
    inventario_->guardarEnArchivo();

    VentasCobro venta(numeroSiguiente_, QString::fromStdString(cajero_), QDateTime::currentDateTime());
    for (const Linea& linea : carrito_.lineas())
        venta.agregar(linea);

    try {
        venta.cobrar(efectivo);
    }
    catch (const std::exception& e) {
        if (mensajeError) *mensajeError = e.what();
        return false;
    }

    ventaCobrada_ = venta;
    ++numeroSiguiente_;
    carrito_.vaciar();
    return true;
}
