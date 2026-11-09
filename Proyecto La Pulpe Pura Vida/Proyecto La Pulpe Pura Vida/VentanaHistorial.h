#pragma once

#include <memory>
#include <vector>

#include <QString>
#include <QWidget>

#include "VentasCobro.h"

class Historial;
class QComboBox;
class QDateEdit;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTextBrowser;

class VentanaHistorial : public QWidget
{
    Q_OBJECT

public:
    explicit VentanaHistorial(std::shared_ptr<Historial> historial, QWidget* parent = nullptr);

signals:
    void volverAlMenu();

private:
    std::shared_ptr<Historial> historial_;

    QStackedWidget* paginas;
    QWidget* paginaHistorial;
    QWidget* paginaDetalle;
    QWidget* paginaError;

    // Pantalla del historial
    QLabel* lblVentasGuardadas;
    QLabel* lblTotalVendido;
    QLabel* lblUltimaVenta;
    QLineEdit* txtBuscar;
    QDateEdit* fechaDesde;
    QDateEdit* fechaHasta;
    QTableWidget* tabla;
    QLabel* lblSinVentas;
    QPushButton* btnFiltrar;
    QPushButton* btnLimpiar;
    QPushButton* btnVolverMenu;

    // Pantalla de error de carga
    QLabel* lblErrorCarga;
    QPushButton* btnReintentarCarga;
    QPushButton* btnVolverError;

    // Pantalla de detalle
    QLabel* lblDetalleTitulo;
    QLabel* lblDetalleInfo;
    QLabel* lblDetalleCajero;
    QLabel* lblDetalleEstado;
    QComboBox* cmbFormato;
    QTableWidget* tablaDetalle;
    QLabel* lblDetalleTotal;
    QLabel* lblDetallePago;
    QLabel* lblDetalleCambio;
    QTextBrowser* vistaPrevia;
    QFrame* bannerMensaje;
    QLabel* lblMensaje;
    QPushButton* btnReintentarImpresion;
    QPushButton* btnImprimir;
    QPushButton* btnGuardar;
    QPushButton* btnVolverHistorial;

    VentasCobro ventaActual_;
    bool hayVentaActual_ = false;

    void construirPaginaHistorial();
    void construirPaginaDetalle();
    void construirPaginaError();
    void aplicarEstilos();

    void cargarHistorial();
    void llenarTabla(const std::vector<VentasCobro>& ventas);
    void actualizarMetricas();
    void limpiarFiltros();
    void aplicarFiltro();

    void mostrarDetalle(int numero);
    void actualizarVistaPrevia();
    void imprimirTicket();
    void guardarCopia();

    void mostrarMensaje(const QString& texto, const QString& tipo, bool permitirReintento = true);
    void ocultarMensaje();
};

