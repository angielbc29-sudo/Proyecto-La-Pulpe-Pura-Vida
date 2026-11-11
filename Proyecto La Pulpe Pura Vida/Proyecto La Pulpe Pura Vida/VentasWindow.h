#pragma once

#include <memory>
#include <string>

#include <QWidget>

#include "ControladorVentas.h"
#include "GestorInventario.h"

class Historial;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTableWidget;

// VentasWindow: pantalla de Ventas y Cobro con estilo de punto de venta.
// El cajero busca productos por codigo o nombre, arma el carrito, cobra en
// efectivo y la venta queda registrada en el historial (ventas.json).
class VentasWindow : public QWidget
{
    Q_OBJECT

public:
    explicit VentasWindow(std::shared_ptr<Historial> historial, QWidget* parent = nullptr);
    ~VentasWindow() override;

signals:
    void volverAlMenu();

private:
    GestorInventario* inventario_;     // [VENTAS] Inventario cargado desde productos.json.
    ControladorVentas controlador_;    // [VENTAS] Logica de carrito, IVA, cobro y registro.

    QLineEdit* txtBuscar;
    QListWidget* listaResultados;
    QPushButton* btnAgregar;
    QTableWidget* tablaCarrito;
    QPushButton* btnVaciar;
    QLabel* lblSubtotal;
    QLabel* lblIva;
    QLabel* lblTotal;
    QDoubleSpinBox* spinEfectivo;
    QLabel* lblCambio;
    QPushButton* btnCobrar;
    QPushButton* btnVolverMenu;

    void construirInterfaz();
    QWidget* crearPanelPrincipal();
    void aplicarEstilos();
    void actualizarVista();
    void actualizarResultadosBusqueda();

    QWidget* crearAccionesFila(const std::string& codigo);

    void agregarAlCarrito();
    void agregarResultadoSeleccionado();
    void agregarProducto(const std::string& codigo);
    void agregarUnidad(const std::string& codigo);
    void quitarUnidadDe(const std::string& codigo);
    void quitarProductoDe(const std::string& codigo);
    void vaciarCarrito();
    void cobrar();

    std::string codigoResultadoSeleccionado() const;
};
