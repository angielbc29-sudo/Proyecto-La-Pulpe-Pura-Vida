#pragma once
#include <QString>
#include "VentasCobro.h"   // [VENTAS]

class Ticket {
public:
    explicit Ticket(const QString& negocio = "La Pulpe") : negocio_(negocio) {}
    virtual ~Ticket() = default;

    // [VENTAS] Solo LEE la venta; no la guarda (dependencia "lee" del UML).
    virtual QString crear(const VentasCobro& venta) const = 0;
    virtual QString extension() const = 0;   // "txt", "html", "pdf"

    virtual void guardar(const VentasCobro& venta, const QString& ruta) const;

protected:
    QString negocio_;
};
