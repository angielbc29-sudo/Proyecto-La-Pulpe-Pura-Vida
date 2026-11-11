#include "Carrito.h"

#include <QString>

#include "VentasCobro.h"   // [VENTAS] tasa de IVA compartida

// [VENTAS] Agrega una unidad del producto. Si ya esta en el carrito, aumenta
// la cantidad; nunca permite superar el stock disponible.
bool Carrito::agregar(const Producto& producto) {
    const int indice = buscarIndice(producto.getCodigo());

    if (indice >= 0) {
        const Linea& actual = lineas_[indice];
        if (actual.cantidad() + 1 > producto.getStock())
            return false;
        lineas_[indice] = Linea(actual.nombre(), actual.cantidad() + 1, actual.precio(), actual.codigo());
        return true;
    }

    if (producto.getStock() < 1)
        return false;

    lineas_.push_back(Linea(QString::fromStdString(producto.getNombre()),
        1, producto.getPrecio(), QString::fromStdString(producto.getCodigo())));
    return true;
}

// [VENTAS] Quita una unidad de una linea; si queda en cero, elimina la linea.
bool Carrito::quitarUnidad(const std::string& codigo) {
    const int indice = buscarIndice(codigo);
    if (indice < 0)
        return false;

    const Linea& actual = lineas_[indice];
    if (actual.cantidad() > 1)
        lineas_[indice] = Linea(actual.nombre(), actual.cantidad() - 1, actual.precio(), actual.codigo());
    else
        lineas_.erase(lineas_.begin() + indice);
    return true;
}

// [VENTAS] Elimina la linea completa del carrito.
bool Carrito::quitar(const std::string& codigo) {
    const int indice = buscarIndice(codigo);
    if (indice < 0)
        return false;
    lineas_.erase(lineas_.begin() + indice);
    return true;
}

// [VENTAS] Deja el carrito sin ninguna linea.
void Carrito::vaciar() {
    lineas_.clear();
}

bool Carrito::estaVacio() const {
    return lineas_.empty();
}

int Carrito::cantidadArticulos() const {
    int unidades = 0;
    for (const Linea& linea : lineas_)
        unidades += linea.cantidad();
    return unidades;
}

// [VENTAS] Suma de los subtotales de las lineas, sin IVA.
double Carrito::subtotal() const {
    double suma = 0.0;
    for (const Linea& linea : lineas_)
        suma += linea.subtotal();
    return suma;
}

// [VENTAS] IVA del carrito usando la misma tasa que VentasCobro.
double Carrito::iva() const {
    return subtotal() * VentasCobro::tasaIVA();
}

// [VENTAS] Total del carrito: subtotal + IVA.
double Carrito::total() const {
    return subtotal() + iva();
}

const std::vector<Linea>& Carrito::lineas() const {
    return lineas_;
}

// [VENTAS] Devuelve la posicion de un producto en el carrito o -1 si no existe.
int Carrito::buscarIndice(const std::string& codigo) const {
    const QString buscado = QString::fromStdString(codigo);
    for (size_t i = 0; i < lineas_.size(); ++i)
        if (lineas_[i].codigo() == buscado)
            return static_cast<int>(i);
    return -1;
}
