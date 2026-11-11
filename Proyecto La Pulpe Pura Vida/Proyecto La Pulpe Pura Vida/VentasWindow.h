#pragma once

#include <memory>

#include <QWidget>

#include "ControladorVentas.h"
#include "GestorInventario.h"

class Historial;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

// VentasWindow: pantalla de Ventas y Cobro.
// El cajero agrega productos por código, arma el carrito, cobra con efectivo
// y la venta queda registrada en el historial (ventas.json).
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

    QLineEdit* txtCodigo;
    QPushButton* btnAgregar;
    QTableWidget* tablaCarrito;
    QPushButton* btnQuitarUnidad;
    QPushButton* btnQuitarProducto;
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

    void agregarAlCarrito();
    void quitarUnidad();
    void quitarProducto();
    void vaciarCarrito();
    void cobrar();

    int filaSeleccionada() const;
    std::string codigoDeFila(int fila) const;
};
