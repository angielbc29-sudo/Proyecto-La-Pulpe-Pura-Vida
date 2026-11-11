#include "ControladorVentas.h"

#include <algorithm>
#include <exception>
#include <utility>

#include <QDateTime>

#include "Historial.h"

// [VENTAS] Constructor: guarda inventario, historial y cajero.
// Si hay historial, continua la numeracion desde la venta mas alta registrada
// para no repetir numeros aunque falten numeros intermedios.
ControladorVentas::ControladorVentas(GestorInventario* inventario,
    std::shared_ptr<Historial> historial,
    const std::string& cajero)
    : inventario_(inventario), historial_(std::move(historial)), cajero_(cajero)
{
    if (historial_ == nullptr)
        return;

    try
    {
        int maximo = 0;
        for (const VentasCobro& venta : historial_->listar())
            maximo = std::max(maximo, venta.numero());
        numeroSiguiente_ = maximo + 1;
    }
    catch (const std::exception&)
    {
        // [VENTAS] Si el historial esta danado se mantiene el numero 1;
        // al cobrar se volvera a leer y se mostrara el error correspondiente.
        numeroSiguiente_ = 1;
    }
}

// [VENTAS] Busca el producto en el inventario y lo agrega al carrito.
// Respeta el stock y solo permite productos activos.
bool ControladorVentas::agregarProducto(const std::string& codigo) {
    if (inventario_ == nullptr)
        return false;

    Producto* producto = inventario_->buscarPorCodigo(codigo);
    if (producto == nullptr || !producto->getActivo())
        return false;

    return carrito_.agregar(*producto);
}

// [VENTAS] Quita una sola unidad de la linea indicada.
void ControladorVentas::quitarUnidad(const std::string& codigo) {
    carrito_.quitarUnidad(codigo);
}

// [VENTAS] Quita la linea completa del carrito.
void ControladorVentas::quitarProducto(const std::string& codigo) {
    carrito_.quitar(codigo);
}

// [VENTAS] Vacia el carrito por completo.
void ControladorVentas::vaciarCarrito() {
    carrito_.vaciar();
}

// [VENTAS] Subtotal del carrito, sin IVA.
double ControladorVentas::subtotal() const {
    return carrito_.subtotal();
}

// [VENTAS] IVA del carrito (13%).
double ControladorVentas::iva() const {
    return carrito_.iva();
}

// [VENTAS] Total a pagar: subtotal + IVA.
double ControladorVentas::total() const {
    return carrito_.total();
}

// [VENTAS] Indica si el carrito no tiene lineas.
bool ControladorVentas::carritoVacio() const {
    return carrito_.estaVacio();
}

// [VENTAS] Lineas que la interfaz muestra en la tabla del carrito.
const std::vector<Linea>& ControladorVentas::lineasCarrito() const {
    return carrito_.lineas();
}

// [VENTAS] Ultima venta cobrada con exito.
const VentasCobro& ControladorVentas::ventaCobrada() const {
    return ventaCobrada_;
}

// [VENTAS] Permite fijar el numero de la proxima venta (pruebas o ajustes).
void ControladorVentas::fijarNumeroSiguiente(int numero) {
    numeroSiguiente_ = numero > 0 ? numero : 1;
}

// [VENTAS] Cobra en efectivo y registra la venta como una sola operacion.
// Orden: validar -> construir venta -> descontar stock -> guardar inventario
// -> registrar venta. Si un paso falla, se revierte el stock para no dejar
// el inventario rebajado sin una venta en el historial.
bool ControladorVentas::cobrar(double efectivo, std::string* mensajeError) {
    // 1) Validaciones previas. Todavia no se modifica nada.
    if (inventario_ == nullptr) {
        if (mensajeError) *mensajeError = "El inventario no esta disponible.";
        return false;
    }

    if (historial_ == nullptr) {
        if (mensajeError) *mensajeError = "No hay historial disponible para guardar la venta.";
        return false;
    }

    if (carrito_.estaVacio()) {
        if (mensajeError) *mensajeError = "El carrito esta vacio.";
        return false;
    }

    // [VENTAS] Solo se acepta pago en efectivo: debe cubrir el total (con IVA).
    const double totalVenta = carrito_.total();
    if (efectivo < totalVenta) {
        if (mensajeError) *mensajeError = "El efectivo no cubre el total de la venta.";
        return false;
    }

    // 2) Comprobar que haya stock suficiente de cada linea.
    for (const Linea& linea : carrito_.lineas()) {
        if (!inventario_->verificarDisponibilidad(linea.codigo().toStdString(), linea.cantidad())) {
            if (mensajeError) *mensajeError = "Stock insuficiente de " + linea.nombre().toStdString() + ".";
            return false;
        }
    }

    // 3) Construir y validar la venta en memoria (aun no se guarda nada).
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

    // 4) Verificar que el numero no exista antes de tocar el inventario.
    try {
        if (historial_->existe(venta.numero())) {
            if (mensajeError) *mensajeError = "Ya existe la venta N.o " + std::to_string(venta.numero()) + ".";
            return false;
        }
    }
    catch (const std::exception& e) {
        if (mensajeError) *mensajeError = e.what();
        return false;
    }

    // 5) Descontar stock en memoria. Si falla una linea, se devuelve lo ya descontado.
    int descontadas = 0;
    for (const Linea& linea : carrito_.lineas()) {
        if (!inventario_->descontarStock(linea.codigo().toStdString(), linea.cantidad())) {
            for (int i = 0; i < descontadas; ++i)
                devolverStock(carrito_.lineas()[static_cast<size_t>(i)]);
            if (mensajeError) *mensajeError = "No se pudo descontar el stock de " + linea.nombre().toStdString() + ".";
            return false;
        }
        ++descontadas;
    }

    // 6) Guardar el inventario. Si falla, se revierte el stock en memoria.
    if (!inventario_->guardarEnArchivo()) {
        for (const Linea& linea : carrito_.lineas())
            devolverStock(linea);
        inventario_->guardarEnArchivo();   // intentar dejar el archivo como estaba
        if (mensajeError) *mensajeError = "No se pudo guardar el inventario.";
        return false;
    }

    // 7) Registrar la venta en el historial. Si falla, se revierte el stock.
    try {
        historial_->registrar(venta);
    }
    catch (const std::exception& e) {
        for (const Linea& linea : carrito_.lineas())
            devolverStock(linea);
        inventario_->guardarEnArchivo();
        if (mensajeError) *mensajeError = e.what();
        return false;
    }

    // 8) Exito: recien ahora se actualiza el estado del controlador.
    ventaCobrada_ = venta;
    ++numeroSiguiente_;
    carrito_.vaciar();
    return true;
}

// [VENTAS] Devuelve al inventario las unidades de una linea (rollback).
void ControladorVentas::devolverStock(const Linea& linea) {
    Producto* producto = inventario_->buscarPorCodigo(linea.codigo().toStdString());
    if (producto != nullptr)
        producto->agregarStock(linea.cantidad());
}
