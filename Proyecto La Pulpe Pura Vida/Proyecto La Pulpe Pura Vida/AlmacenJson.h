#pragma once
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include "Almacen.h"

class AlmacenJson : public Almacen {
public:
    explicit AlmacenJson(const QString& ruta);

    bool guardar(const VentasCobro& venta) override;
    std::vector<VentasCobro> listar() const override;
    VentasCobro buscar(int numero) const override;
    bool existe(int numero) const override;

private:
    QString ruta_;
    QJsonArray leerArreglo() const;
    void escribirArreglo(const QJsonArray& arreglo) const;
    static QJsonObject aJson(const VentasCobro& venta);
    static VentasCobro deJson(const QJsonObject& obj);
};
