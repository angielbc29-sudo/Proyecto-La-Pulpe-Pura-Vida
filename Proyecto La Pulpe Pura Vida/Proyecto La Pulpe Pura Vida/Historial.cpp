#include "Historial.h"

#include <algorithm>
#include <utility>

#include "Errores.h"
#include "Ticket.h"

Historial::Historial(std::shared_ptr<Almacen> almacen)
    : almacen_(std::move(almacen))
{
    if (!almacen_)
        throw ErrorHistorial("El historial no tiene un almacén configurado.");
}

void Historial::registrar(const VentasCobro& venta)
{
    if (!venta.estaCobrada())
        throw ErrorHistorial("Solo se pueden registrar ventas cobradas.");
    if (venta.lineas().empty())
        throw ErrorHistorial("La venta no tiene líneas.");
    if (venta.total() <= 0.0)
        throw ErrorHistorial("El total de la venta debe ser mayor que cero.");
    if (venta.pago() < venta.total())
        throw ErrorHistorial("El pago no cubre el total de la venta.");
    if (almacen_->existe(venta.numero()))
        throw ErrorHistorial("Ya existe la venta N.º " + QString::number(venta.numero()) + ".");

    almacen_->guardar(venta);
}

std::vector<VentasCobro> Historial::listar() const
{
    return almacen_->listar();
}

VentasCobro Historial::buscar(int numero) const
{
    return almacen_->buscar(numero);
}

bool Historial::existe(int numero) const
{
    return almacen_->existe(numero);
}

int Historial::cantidadVentas() const
{
    return static_cast<int>(almacen_->listar().size());
}

std::vector<VentasCobro> Historial::filtrar(const QString& texto,
    const QDate& desde,
    const QDate& hasta) const
{
    const QString busqueda = texto.trimmed();
    std::vector<VentasCobro> resultado;

    for (const VentasCobro& venta : almacen_->listar())
    {
        const bool coincideTexto =
            busqueda.isEmpty()
            || QString::number(venta.numero()).contains(busqueda, Qt::CaseInsensitive)
            || venta.cajero().contains(busqueda, Qt::CaseInsensitive);

        const QDate fecha = venta.fecha().date();
        const bool coincideDesde = !desde.isValid() || fecha >= desde;
        const bool coincideHasta = !hasta.isValid() || fecha <= hasta;

        if (coincideTexto && coincideDesde && coincideHasta)
            resultado.push_back(venta);
    }

    std::sort(resultado.begin(), resultado.end(),
        [](const VentasCobro& a, const VentasCobro& b)
        {
            if (a.fecha() != b.fecha())
                return a.fecha() > b.fecha();
            return a.numero() > b.numero();
        });

    return resultado;
}

double Historial::totalVendido() const
{
    double total = 0.0;
    for (const VentasCobro& venta : almacen_->listar())
        total += venta.total();
    return total;
}

bool Historial::ultimaVenta(VentasCobro& salida) const
{
    const std::vector<VentasCobro> ventas = almacen_->listar();
    if (ventas.empty())
        return false;

    const auto it = std::max_element(ventas.begin(), ventas.end(),
        [](const VentasCobro& a, const VentasCobro& b)
        {
            if (a.fecha() != b.fecha())
                return a.fecha() < b.fecha();
            return a.numero() < b.numero();
        });

    salida = *it;
    return true;
}

QString Historial::generar(const VentasCobro& venta, const Ticket& ticket) const
{
    return ticket.crear(venta);
}

void Historial::imprimir(const VentasCobro& venta, const Ticket& ticket, const QString& ruta) const
{
    ticket.guardar(venta, ruta);
}
