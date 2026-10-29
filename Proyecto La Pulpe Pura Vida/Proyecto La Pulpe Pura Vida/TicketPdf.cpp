#include "TicketPdf.h"
#include "TicketHtml.h"
#include "Errores.h"
#include <QFile>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QMarginsF>
#include <QTextDocument>

QString TicketPdf::extension() const { return "pdf"; }

QString TicketPdf::crear(const VentasCobro& venta) const {
    return TicketHtml(negocio_).crear(venta);
}

void TicketPdf::guardar(const VentasCobro& venta, const QString& ruta) const {
    QFile prueba(ruta);   // comprobar que se puede escribir antes de generar
    if (!prueba.open(QIODevice::WriteOnly))
        throw ErrorHistorial("No se pudo crear el PDF: " + prueba.errorString());
    prueba.close();

    QPdfWriter pdf(ruta);
    pdf.setPageSize(QPageSize(QPageSize::A5));
    pdf.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    pdf.setTitle("Ticket " + QString::number(venta.numero()));   // [VENTAS] numero()

    QTextDocument doc;
    doc.setHtml(crear(venta));
    doc.print(&pdf);
}