#include "VentasCobro.h"

#include <stdexcept>

VentasCobro::VentasCobro()
    : fecha_(QDateTime::currentDateTime())
{
}

VentasCobro::VentasCobro(int numero, const QString& cajero, const QDateTime& fecha)
    : numero_(numero), fecha_(fecha), cajero_(cajero)
{
    if (!fecha_.isValid())
        fecha_ = QDateTime::currentDateTime();
}

int VentasCobro::numero() const
{
    return numero_;
}

const QDateTime& VentasCobro::fecha() const
{
    return fecha_;
}

const QString& VentasCobro::cajero() const
{
    return cajero_;
}

const std::vector<Linea>& VentasCobro::lineas() const
{
    return lineas_;
}

double VentasCobro::total() const
{
    double resultado = 0.0;
    for (const Linea& linea : lineas_)
        resultado += linea.subtotal();
    return resultado;
}

double VentasCobro::pago() const
{
    return pago_;
}

double VentasCobro::cambio() const
{
    return cambio_;
}

bool VentasCobro::estaCobrada() const
{
    return cobrada_;
}

void VentasCobro::agregar(const Linea& linea)
{
    if (cobrada_)
        throw std::logic_error("No se puede modificar una venta cobrada.");
    if (linea.nombre().trimmed().isEmpty())
        throw std::invalid_argument("La línea debe tener un nombre.");
    if (linea.cantidad() <= 0 || linea.precio() < 0.0)
        throw std::invalid_argument("La cantidad y el precio deben ser válidos.");

    lineas_.push_back(linea);
}

void VentasCobro::cobrar(double pago)
{
    if (cobrada_)
        throw std::logic_error("La venta ya está cobrada.");
    if (lineas_.empty())
        throw std::logic_error("No se puede cobrar una venta sin líneas.");
    if (pago < total())
        throw std::invalid_argument("El pago no cubre el total de la venta.");

    pago_ = pago;
    cambio_ = pago_ - total();
    cobrada_ = true;
}
