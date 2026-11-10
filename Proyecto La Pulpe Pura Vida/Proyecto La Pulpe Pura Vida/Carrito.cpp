#include "Carrito.h"

#include <QString>

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

bool Carrito::quitar(const std::string& codigo) {
    const int indice = buscarIndice(codigo);
    if (indice < 0)
        return false;
    lineas_.erase(lineas_.begin() + indice);
    return true;
}

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

double Carrito::subtotal() const {
    double suma = 0.0;
    for (const Linea& linea : lineas_)
        suma += linea.subtotal();
    return suma;
}

double Carrito::iva() const {
    return subtotal() * 0.13;
}

double Carrito::total() const {
    return subtotal() + iva();
}

const std::vector<Linea>& Carrito::lineas() const {
    return lineas_;
}

int Carrito::buscarIndice(const std::string& codigo) const {
    const QString buscado = QString::fromStdString(codigo);
    for (size_t i = 0; i < lineas_.size(); ++i)
        if (lineas_[i].codigo() == buscado)
            return static_cast<int>(i);
    return -1;
}
