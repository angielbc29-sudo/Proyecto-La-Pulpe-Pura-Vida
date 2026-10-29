#include "TicketTxt.h"
#include "Formato.h"
#include "Linea.h"   // [VENTAS] Pendiente creaacion de la clase Linea.h

QString TicketTxt::extension() const { return "txt"; }

// [VENTAS] Lee: numero(), fecha(), cajero(), lineas(), total(), pago(), cambio().
QString TicketTxt::crear(const VentasCobro& v) const {
    const int W = 30;
    const QString sep(W, QChar('-'));
    QString t;
    t += Formato::centrar(negocio_.toUpper(), W) + "\n";
    t += Formato::centrar("La Pulpe Pura Vida", W) + "\n";
    t += sep + "\n";
    t += "TICKET / FACTURA N.º " + Formato::numero(v.numero()) + "\n";
    t += Formato::fila(v.fecha().toString("dd/MM/yyyy"), v.fecha().toString("HH:mm"), W) + "\n";
    t += "Cajero: " + v.cajero() + "\n";
    t += sep + "\n";
    for (const Linea& l : v.lineas()) {
        t += l.nombre() + "\n";
        const QString izq = QString("  %1 x %2").arg(l.cantidad()).arg(Formato::sinSimbolo(l.precio()));
        t += Formato::fila(izq, Formato::sinSimbolo(l.subtotal()), W - 2) + "\n";
    }
    t += sep + "\n";
    t += Formato::fila("TOTAL", Formato::colones(v.total()), W) + "\n";
    t += Formato::fila("Efectivo", Formato::colones(v.pago()), W) + "\n";
    t += Formato::fila("Cambio", Formato::colones(v.cambio()), W) + "\n";
    t += sep + "\n";
    t += Formato::centrar("¡Gracias por su compra!", W) + "\n";
    return t;
}