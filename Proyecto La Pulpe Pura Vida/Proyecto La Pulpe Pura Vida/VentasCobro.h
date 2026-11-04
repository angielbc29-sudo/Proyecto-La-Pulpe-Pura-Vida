#pragma once

#include <QDateTime>
#include <QString>
#include <vector>

#include "Linea.h"

class VentasCobro
{
public:
    VentasCobro();
    VentasCobro(int numero, const QString& cajero, const QDateTime& fecha);

    int numero() const;
    const QDateTime& fecha() const;
    const QString& cajero() const;
    const std::vector<Linea>& lineas() const;

    double total() const;
    double pago() const;
    double cambio() const;
    bool estaCobrada() const;

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
