#include "TicketHtml.h"
#include "Formato.h"
#include "Linea.h"   // [VENTAS] 

QString TicketHtml::extension() const { return "html"; }

// [VENTAS] Lee los mismos datos que TicketTxt.

QString TicketHtml::crear(const VentasCobro& v) const {
    QString h;
    h += "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
    h += "<title>Ticket N.º " + Formato::numero(v.numero()) + "</title></head>";
    h += "<body style=\"font-family:'Courier New',monospace; font-size:10pt;\">";
    h += "<h2 align=\"center\">" + negocio_.toHtmlEscaped().toUpper() + "</h2><hr>";
    h += "<p><b>TICKET / FACTURA N.º " + Formato::numero(v.numero()) + "</b><br>" +
        v.fecha().toString("dd/MM/yyyy") + " &nbsp; " + v.fecha().toString("HH:mm") +
        "<br>Cajero: " + v.cajero().toHtmlEscaped() + "</p><hr>";

    h += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\">";
    h += "<tr><th align=\"left\">Producto</th><th>Cant.</th>"
        "<th align=\"right\">Precio</th><th align=\"right\">Subtotal</th></tr>";
    for (const Linea& l : v.lineas()) {
        h += "<tr><td>" + l.nombre().toHtmlEscaped() + "</td>"
            "<td align=\"center\">" + QString::number(l.cantidad()) + "</td>"
            "<td align=\"right\">" + Formato::sinSimbolo(l.precio()) + "</td>"
            "<td align=\"right\">" + Formato::sinSimbolo(l.subtotal()) + "</td></tr>";
    }
    h += "</table><hr>";

    auto fila = [](const QString& n, const QString& m) {
        return "<tr><td><b>" + n + "</b></td><td align=\"right\"><b>" + m + "</b></td></tr>";
        };
    h += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\">";
    h += fila("Subtotal", Formato::colones(v.subtotal()));
    h += fila("IVA (13%)", Formato::colones(v.iva()));
    h += fila("TOTAL", Formato::colones(v.total()));
    h += fila("Efectivo", Formato::colones(v.pago()));
    h += fila("Cambio", Formato::colones(v.cambio()));
    h += "</table><hr>";
    h += "<p align=\"center\">¡Gracias por su compra!</p></body></html>";
    return h;
}