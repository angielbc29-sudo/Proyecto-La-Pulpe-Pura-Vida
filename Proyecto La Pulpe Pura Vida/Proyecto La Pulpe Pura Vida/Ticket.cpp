#include "Ticket.h"
#include "Errores.h"
#include <QSaveFile>

void Ticket::guardar(const VentasCobro& venta, const QString& ruta) const {
    QSaveFile f(ruta);
    if (!f.open(QIODevice::WriteOnly))
        throw ErrorHistorial("No se pudo crear el archivo: " + f.errorString());
    f.write(crear(venta).toUtf8());
    if (!f.commit())
        throw ErrorHistorial("No se pudo guardar el archivo del ticket.");
}