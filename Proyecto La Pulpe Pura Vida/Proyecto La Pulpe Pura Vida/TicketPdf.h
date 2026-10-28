#pragma once
#include "Ticket.h"

class TicketPdf : public Ticket {
public:
    using Ticket::Ticket;
    QString crear(const VentasCobro& venta) const override;   // HTML que se dibuja en el PDF
    QString extension() const override;
    void guardar(const VentasCobro& venta, const QString& ruta) const override;  // genera el PDF real
};