#pragma once

#include <QDateTime>
#include <QString>
#include <vector>

#include "Linea.h"

// [VENTAS] Venta ya cobrada: numero, fecha, cajero, lineas y datos del pago.
// Los montos se calculan siempre a partir de las lineas:
//   subtotal = suma de (cantidad * precio)
//   iva      = subtotal * tasaIVA()
//   total    = subtotal + iva
class VentasCobro
{
public:
    // [VENTAS] Tasa de IVA compartida con Carrito (un solo lugar para cambiarla).
    static constexpr double tasaIVA() { return 0.13; }

    VentasCobro();
    VentasCobro(int numero, const QString& cajero, const QDateTime& fecha);

    int numero() const;
    const QDateTime& fecha() const;
    const QString& cajero() const;
    const std::vector<Linea>& lineas() const;

    // [VENTAS] Montos calculados. total() incluye el IVA.
    double subtotal() const;
    double iva() const;
    double total() const;

    // [VENTAS] Datos del cobro en efectivo.
    double pago() const;
    double cambio() const;
    bool estaCobrada() const;

    // [VENTAS] Solo se puede modificar mientras la venta no este cobrada.
    void agregar(const Linea& linea);
    void cobrar(double pago);

private:
    int numero_ = 0;
    QDateTime fecha_;
    QString cajero_;
    std::vector<Linea> lineas_;
    double pago_ = 0.0;
    double cambio_ = 0.0;
    bool cobrada_ = false;
};
