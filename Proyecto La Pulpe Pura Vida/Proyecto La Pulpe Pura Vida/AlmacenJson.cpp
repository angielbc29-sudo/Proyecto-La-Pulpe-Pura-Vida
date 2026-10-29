#include "AlmacenJson.h"
#include "Errores.h"
#include "Linea.h"   // [VENTAS]
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonParseError>

AlmacenJson::AlmacenJson(const QString& ruta) : ruta_(ruta) {
    if (ruta_.trimmed().isEmpty())
        throw ErrorHistorial("La ruta del archivo de ventas está vacía.");

    const QFileInfo info(ruta_);
    if (!QDir().mkpath(info.absolutePath()))
        throw ErrorHistorial("No se pudo crear la carpeta: " + info.absolutePath());

    // Primera vez: se crea vacío y el historial aparece sin ventas.
    if (!info.exists())
        escribirArreglo(QJsonArray());
}

QJsonArray AlmacenJson::leerArreglo() const {
    QFile f(ruta_);
    if (!f.open(QIODevice::ReadOnly))
        throw ErrorHistorial("No se pudo abrir ventas.json: " + f.errorString());

    const QByteArray datos = f.readAll();
    if (datos.trimmed().isEmpty()) return QJsonArray();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(datos, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        throw ErrorHistorial("El archivo ventas.json está dañado: " + err.errorString());

    return doc.object().value("ventas").toArray();
}

void AlmacenJson::escribirArreglo(const QJsonArray& arreglo) const {
    QSaveFile f(ruta_);
    if (!f.open(QIODevice::WriteOnly))
        throw ErrorHistorial("No se pudo escribir ventas.json: " + f.errorString());

    QJsonObject raiz;
    raiz["ventas"] = arreglo;
    const QByteArray datos = QJsonDocument(raiz).toJson(QJsonDocument::Indented);
    if (f.write(datos) != datos.size())
        throw ErrorHistorial("No se pudo escribir completo ventas.json.");
    if (!f.commit())
        throw ErrorHistorial("No se pudo guardar ventas.json: " + f.errorString());
}

// [VENTAS] Convierte una Venta a JSON. Lee numero(), fecha(), cajero(), lineas()
// (nombre, cantidad, precio), total(), pago() y cambio(). Si Ventas cambia el
// nombre de algún método, se ajusta SOLO aquí.
QJsonObject AlmacenJson::aJson(const VentasCobro& v) {
    QJsonArray lineas;
    for (const Linea& l : v.lineas()) {
        QJsonObject o;
        o["nombre"] = l.nombre();
        o["cantidad"] = l.cantidad();
        o["precio"] = l.precio();
        lineas.append(o);
    }
    QJsonObject obj;
    obj["numero"] = v.numero();
    obj["fecha"] = v.fecha().toString(Qt::ISODate);
    obj["cajero"] = v.cajero();
    obj["lineas"] = lineas;
    obj["total"] = v.total();
    obj["pago"] = v.pago();
    obj["cambio"] = v.cambio();
    return obj;
}

// [VENTAS] Reconstruye una Venta desde el JSON: crea Venta, agrega cada Linea
// y la cobra para dejarla bloqueada. Si Ventas arma sus objetos distinto,
// se ajusta SOLO aquí.
VentasCobro AlmacenJson::deJson(const QJsonObject& obj) {
    try {
        VentasCobro v(obj["numero"].toInt(),
            obj["cajero"].toString(),
            QDateTime::fromString(obj["fecha"].toString(), Qt::ISODate));
        const QJsonArray lineas = obj["lineas"].toArray();
        for (const QJsonValue& val : lineas) {
            const QJsonObject o = val.toObject();
            v.agregar(Linea(o["nombre"].toString(), o["cantidad"].toInt(), o["precio"].toDouble()));
        }
        v.cobrar(obj["pago"].toDouble());
        return v;
    }
    catch (const std::exception& e) {
        throw ErrorHistorial("Registro dañado en ventas.json: " + QString::fromUtf8(e.what()));
    }
}

bool AlmacenJson::guardar(const VentasCobro& venta) {
    if (!venta.estaCobrada())   // [VENTAS]
        throw ErrorHistorial("Solo se pueden guardar ventas cobradas.");

    QJsonArray arreglo = leerArreglo();
    for (const QJsonValue& val : arreglo)
        if (val.toObject()["numero"].toInt() == venta.numero())
            throw ErrorHistorial("Ya existe la venta N.º " + QString::number(venta.numero()) + ".");

    arreglo.append(aJson(venta));
    escribirArreglo(arreglo);
    return true;
}

std::vector<VentasCobro> AlmacenJson::listar() const {
    std::vector<VentasCobro> res;
    const QJsonArray arreglo = leerArreglo();
    for (const QJsonValue& val : arreglo)
        res.push_back(deJson(val.toObject()));
    return res;
}

VentasCobro AlmacenJson::buscar(int numero) const {
    const QJsonArray arreglo = leerArreglo();
    for (const QJsonValue& val : arreglo)
        if (val.toObject()["numero"].toInt() == numero)
            return deJson(val.toObject());
    throw ErrorHistorial("No existe la venta N.º " + QString::number(numero) + ".");
}

bool AlmacenJson::existe(int numero) const {
    const QJsonArray arreglo = leerArreglo();
    for (const QJsonValue& val : arreglo)
        if (val.toObject()["numero"].toInt() == numero) return true;
    return false;
}