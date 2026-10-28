#pragma once
#include "Ticket.h"

class TicketTxt : public Ticket {
public:
    using Ticket::Ticket;
    QString crear(const VentasCobro& venta) const override;
    QString extension() const override;
};
