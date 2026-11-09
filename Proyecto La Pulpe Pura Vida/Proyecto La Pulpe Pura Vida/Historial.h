#pragma once

#include <memory>
#include <vector>

#include <QDate>
#include <QString>

#include "Almacen.h"
#include "VentasCobro.h"

class Ticket;

// Lógica del módulo de Comprobantes e Historial.
// No conoce la interfaz: valida, filtra, totaliza e imprime.
class Historial
{
public:
    explicit Historial(std::shared_ptr<Almacen> almacen);

    // Registra una venta ya cobrada (valida antes de guardar).
    void registrar(const VentasCobro& venta);

    std::vector<VentasCobro> listar() const;
    VentasCobro buscar(int numero) const;
    bool existe(int numero) const;
    int cantidadVentas() const;

    std::vector<VentasCobro> filtrar(const QString& texto,
        const QDate& desde,
        const QDate& hasta) const;

    double totalVendido() const;
    bool ultimaVenta(VentasCobro& salida) const;

    QString generar(const VentasCobro& venta, const Ticket& ticket) const;
    void imprimir(const VentasCobro& venta, const Ticket& ticket, const QString& ruta) const;

private:
    std::shared_ptr<Almacen> almacen_;
};

