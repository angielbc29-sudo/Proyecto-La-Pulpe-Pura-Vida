#include "VentasWindow.h"

#include <utility>

#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "Errores.h"
#include "Formato.h"
#include "Historial.h"
#include "Sesion.h"
#include "Usuario.h"

namespace
{
    std::string cajeroActual()
    {
        const std::shared_ptr<Usuario> usuario = Sesion::getInstancia().getUsuarioActual();
        if (usuario == nullptr)
            return "Cajero";
        if (!usuario->getNombreCompleto().empty())
            return usuario->getNombreCompleto();
        return usuario->getUsername();
    }

    QTableWidgetItem* crearItem(const QString& texto, bool centrado)
    {
        QTableWidgetItem* item = new QTableWidgetItem(texto);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        if (centrado)
            item->setTextAlignment(Qt::AlignCenter);
        return item;
    }

    QFrame* crearTarjetaTotal(QWidget* padre, const QString& etiqueta,
        QLabel*& valor, const QString& color)
    {
        QFrame* tarjeta = new QFrame(padre);
        tarjeta->setObjectName("tarjetaMetrica");

        QVBoxLayout* layout = new QVBoxLayout(tarjeta);
        layout->setContentsMargins(14, 10, 14, 10);
        layout->setSpacing(2);

        QLabel* lblEtiqueta = new QLabel(etiqueta, tarjeta);
        lblEtiqueta->setObjectName("lblEtiquetaMetrica");

        valor = new QLabel("—", tarjeta);
        valor->setObjectName("lblValorMetrica");
        valor->setStyleSheet("font-size: 22px; font-weight: 900; color: " + color + ";");

        layout->addWidget(lblEtiqueta);
        layout->addWidget(valor);
        return tarjeta;
    }
}

VentasWindow::VentasWindow(std::shared_ptr<Historial> historial, QWidget* parent)
    : QWidget(parent),
    inventario_(new GestorInventario()),
    controlador_(inventario_, std::move(historial), cajeroActual())
{
    setWindowTitle("La Pulpe Pura Vida - Ventas y Cobro");
    setAttribute(Qt::WA_StyledBackground, true);

    // [VENTAS] Carga el inventario guardado y prepara la pantalla.
    inventario_->cargarDesdeArchivo("productos.json");

    construirInterfaz();
    aplicarEstilos();

    connect(btnVolverMenu, &QPushButton::clicked, this, &VentasWindow::volverAlMenu);
    connect(btnAgregar, &QPushButton::clicked, this, &VentasWindow::agregarAlCarrito);
    connect(txtCodigo, &QLineEdit::returnPressed, this, &VentasWindow::agregarAlCarrito);
    connect(btnQuitarUnidad, &QPushButton::clicked, this, &VentasWindow::quitarUnidad);
    connect(btnQuitarProducto, &QPushButton::clicked, this, &VentasWindow::quitarProducto);
    connect(btnVaciar, &QPushButton::clicked, this, &VentasWindow::vaciarCarrito);
    connect(btnCobrar, &QPushButton::clicked, this, &VentasWindow::cobrar);

    actualizarVista();
    txtCodigo->setFocus();
}

VentasWindow::~VentasWindow()
{
    delete inventario_;
}

// ---------------- Interfaz ----------------

void VentasWindow::construirInterfaz()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->addWidget(crearPanelPrincipal());
}

QWidget* VentasWindow::crearPanelPrincipal()
{
    QWidget* panel = new QWidget(this);
    panel->setObjectName("panelVentas");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(35, 25, 35, 25);
    layout->setSpacing(14);

    // Encabezado: volver + título + cajero
    QHBoxLayout* layoutEncabezado = new QHBoxLayout();
    btnVolverMenu = new QPushButton("← Menú Principal", panel);
    btnVolverMenu->setObjectName("btnVolver");
    btnVolverMenu->setCursor(Qt::PointingHandCursor);

    QLabel* lblTitulo = new QLabel("Ventas y Cobro", panel);
    lblTitulo->setObjectName("lblTituloPantalla");

    QLabel* lblCajero = new QLabel("Cajero: " + QString::fromStdString(cajeroActual()), panel);
    lblCajero->setObjectName("lblTag");

    layoutEncabezado->addWidget(btnVolverMenu);
    layoutEncabezado->addStretch();
    layoutEncabezado->addWidget(lblTitulo);
    layoutEncabezado->addStretch();
    layoutEncabezado->addWidget(lblCajero);

    layout->addLayout(layoutEncabezado);

    // Fila de captura de código
    QHBoxLayout* layoutCodigo = new QHBoxLayout();
    QLabel* lblCodigo = new QLabel("Código del producto:", panel);
    txtCodigo = new QLineEdit(panel);
    txtCodigo->setPlaceholderText("Escriba el código y presione Enter");
    txtCodigo->setMaximumWidth(320);

    btnAgregar = new QPushButton("Agregar al carrito", panel);
    btnAgregar->setObjectName("btnPrimario");
    btnAgregar->setCursor(Qt::PointingHandCursor);

    layoutCodigo->addWidget(lblCodigo);
    layoutCodigo->addWidget(txtCodigo, 1);
    layoutCodigo->addWidget(btnAgregar);

    layout->addLayout(layoutCodigo);

    // Tabla del carrito
    tablaCarrito = new QTableWidget(0, 5, panel);
    tablaCarrito->setHorizontalHeaderLabels({ "Código", "Producto", "Cant.", "Precio", "Subtotal" });
    tablaCarrito->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tablaCarrito->verticalHeader()->setVisible(false);
    tablaCarrito->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaCarrito->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(tablaCarrito, 1);

    // Botones sobre el carrito
    QHBoxLayout* layoutBotonesCarrito = new QHBoxLayout();
    btnQuitarUnidad = new QPushButton("− Quitar unidad", panel);
    btnQuitarUnidad->setObjectName("btnSuave");
    btnQuitarUnidad->setCursor(Qt::PointingHandCursor);

    btnQuitarProducto = new QPushButton("× Quitar producto", panel);
    btnQuitarProducto->setObjectName("btnSuave");
    btnQuitarProducto->setCursor(Qt::PointingHandCursor);

    btnVaciar = new QPushButton("Vaciar carrito", panel);
    btnVaciar->setObjectName("btnSuave");
    btnVaciar->setCursor(Qt::PointingHandCursor);

    layoutBotonesCarrito->addWidget(btnQuitarUnidad);
    layoutBotonesCarrito->addWidget(btnQuitarProducto);
    layoutBotonesCarrito->addStretch();
    layoutBotonesCarrito->addWidget(btnVaciar);

    layout->addLayout(layoutBotonesCarrito);

    // Panel de totales y cobro
    QHBoxLayout* layoutCobro = new QHBoxLayout();
    layoutCobro->setSpacing(12);

    QFrame* tarjetaSubtotal = crearTarjetaTotal(panel, "Subtotal", lblSubtotal, "#3b2a1a");
    QFrame* tarjetaIva = crearTarjetaTotal(panel, "IVA (13%)", lblIva, "#8a6a4f");
    QFrame* tarjetaTotal = crearTarjetaTotal(panel, "Total a pagar", lblTotal, "#e67e22");

    layoutCobro->addWidget(tarjetaSubtotal);
    layoutCobro->addWidget(tarjetaIva);
    layoutCobro->addWidget(tarjetaTotal);
    layoutCobro->addStretch();

    // Efectivo + botón cobrar + cambio
    QFrame* tarjetaCobro = new QFrame(panel);
    tarjetaCobro->setObjectName("tarjetaCobro");
    QVBoxLayout* layoutTarjetaCobro = new QVBoxLayout(tarjetaCobro);
    layoutTarjetaCobro->setContentsMargins(16, 12, 16, 12);
    layoutTarjetaCobro->setSpacing(8);

    QLabel* lblEfectivo = new QLabel("Efectivo recibido:", tarjetaCobro);
    lblEfectivo->setObjectName("lblEtiquetaMetrica");

    spinEfectivo = new QDoubleSpinBox(tarjetaCobro);
    spinEfectivo->setPrefix("₡ ");
    spinEfectivo->setDecimals(0);
    spinEfectivo->setSingleStep(100);
    spinEfectivo->setMaximum(10000000);

    btnCobrar = new QPushButton("Cobrar venta", tarjetaCobro);
    btnCobrar->setObjectName("btnCobrar");
    btnCobrar->setCursor(Qt::PointingHandCursor);

    lblCambio = new QLabel("Cambio: —", tarjetaCobro);
    lblCambio->setObjectName("lblCambio");

    layoutTarjetaCobro->addWidget(lblEfectivo);
    layoutTarjetaCobro->addWidget(spinEfectivo);
    layoutTarjetaCobro->addWidget(btnCobrar);
    layoutTarjetaCobro->addWidget(lblCambio);

    layoutCobro->addWidget(tarjetaCobro);

    layout->addLayout(layoutCobro);

    return panel;
}

void VentasWindow::aplicarEstilos()
{
    setStyleSheet(
        "VentasWindow { background-color: #fff8ee; }"
        "QWidget { font-family: 'Segoe UI'; font-size: 14px; color: #3b2a1a; }"
        "QLabel { background: transparent; }"
        "QLabel#lblTituloPantalla { font-size: 26px; font-weight: bold; }"
        "QLabel#lblTag { background-color: #fff3e0; color: #8a6a4f; border: 1px solid #f0d9b5; border-radius: 10px; padding: 5px 12px; font-size: 11px; font-weight: bold; }"
        "QLabel#lblEtiquetaMetrica { font-size: 11px; font-weight: bold; color: #8a6a4f; }"
        "QFrame#tarjetaMetrica { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QFrame#tarjetaCobro { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QPushButton#btnVolver { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }"
        "QPushButton#btnVolver:hover { background-color: #fffaf3; border: 1px solid #e67e22; }"
        "QPushButton#btnPrimario { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 9px 18px; font-weight: bold; }"
        "QPushButton#btnPrimario:hover { background-color: #cf6d17; }"
        "QPushButton#btnSuave { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 8px 14px; font-weight: bold; }"
        "QPushButton#btnSuave:hover { background-color: #fffaf3; border: 1px solid #e67e22; }"
        "QPushButton#btnCobrar { background-color: #27ae60; color: #ffffff; border: none; border-radius: 8px; padding: 11px 22px; font-weight: bold; }"
        "QPushButton#btnCobrar:hover { background-color: #1e8449; }"
        "QPushButton#btnCobrar:disabled { background-color: #d5cdbf; color: #ffffff; }"
        "QLineEdit, QDoubleSpinBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 7px; padding: 7px 10px; }"
        "QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; border-bottom: 1px solid #f0d9b5; padding: 8px; }"
        "QLabel#lblCambio { font-size: 15px; font-weight: bold; color: #27ae60; }"
    );
}

// ---------------- Estado de la pantalla ----------------

// [VENTAS] Refresca la tabla del carrito, los totales (subtotal, IVA, total)
// y el estado de los botones segun haya lineas o seleccion.
void VentasWindow::actualizarVista()
{
    const std::vector<Linea>& lineas = controlador_.lineasCarrito();

    tablaCarrito->setRowCount(static_cast<int>(lineas.size()));
    for (int i = 0; i < static_cast<int>(lineas.size()); ++i)
    {
        const Linea& linea = lineas[static_cast<size_t>(i)];
        tablaCarrito->setItem(i, 0, crearItem(linea.codigo(), true));
        tablaCarrito->setItem(i, 1, crearItem(linea.nombre(), false));
        tablaCarrito->setItem(i, 2, crearItem(QString::number(linea.cantidad()), true));
        tablaCarrito->setItem(i, 3, crearItem(Formato::sinSimbolo(linea.precio()), true));
        tablaCarrito->setItem(i, 4, crearItem(Formato::sinSimbolo(linea.subtotal()), true));
    }

    lblSubtotal->setText(Formato::colones(controlador_.subtotal()));
    lblIva->setText(Formato::colones(controlador_.iva()));
    lblTotal->setText(Formato::colones(controlador_.total()));

    const bool hayLineas = !controlador_.carritoVacio();
    btnCobrar->setEnabled(hayLineas);
    btnVaciar->setEnabled(hayLineas);

    const bool haySeleccion = filaSeleccionada() >= 0;
    btnQuitarUnidad->setEnabled(haySeleccion);
    btnQuitarProducto->setEnabled(haySeleccion);
}

// ---------------- Acciones del carrito ----------------

// [VENTAS] Lee el codigo escrito, valida que el producto exista y lo agrega.
void VentasWindow::agregarAlCarrito()
{
    const QString texto = txtCodigo->text().trimmed();
    if (texto.isEmpty())
        return;

    const std::string codigo = texto.toStdString();
    const Producto* producto = inventario_->buscarPorCodigo(codigo);

    if (producto == nullptr)
    {
        QMessageBox::warning(this, "Producto no encontrado",
            "No existe un producto con el código \"" + texto + "\".");
        txtCodigo->selectAll();
        return;
    }

    if (!controlador_.agregarProducto(codigo))
    {
        QMessageBox::warning(this, "Stock insuficiente",
            "No hay más unidades disponibles de \"" + QString::fromStdString(producto->getNombre()) + "\".");
        txtCodigo->selectAll();
        return;
    }

    actualizarVista();
    txtCodigo->clear();
    txtCodigo->setFocus();
}

// [VENTAS] Devuelve la fila seleccionada de la tabla o -1 si no hay ninguna.
int VentasWindow::filaSeleccionada() const
{
    const int fila = tablaCarrito->currentRow();
    if (fila < 0 || fila >= tablaCarrito->rowCount())
        return -1;
    return fila;
}

// [VENTAS] Obtiene el codigo del producto de una fila de la tabla.
std::string VentasWindow::codigoDeFila(int fila) const
{
    const QTableWidgetItem* item = tablaCarrito->item(fila, 0);
    return item != nullptr ? item->text().toStdString() : std::string();
}

// [VENTAS] Quita una unidad de la linea seleccionada.
void VentasWindow::quitarUnidad()
{
    const int fila = filaSeleccionada();
    if (fila < 0)
        return;
    controlador_.quitarUnidad(codigoDeFila(fila));
    actualizarVista();
}

// [VENTAS] Quita la linea completa seleccionada.
void VentasWindow::quitarProducto()
{
    const int fila = filaSeleccionada();
    if (fila < 0)
        return;
    controlador_.quitarProducto(codigoDeFila(fila));
    actualizarVista();
}

// [VENTAS] Pide confirmacion y vacia todo el carrito.
void VentasWindow::vaciarCarrito()
{
    if (controlador_.carritoVacio())
        return;
    const QMessageBox::StandardButton respuesta = QMessageBox::question(this,
        "Vaciar carrito", "¿Seguro que desea vaciar el carrito?",
        QMessageBox::Yes | QMessageBox::No);

    if (respuesta == QMessageBox::Yes)
    {
        controlador_.vaciarCarrito();
        actualizarVista();
    }
}

// ---------------- Cobro ----------------

// [VENTAS] Ejecuta el cobro en efectivo. El controlador valida, descuenta
// inventario y registra la venta; aqui solo se muestra el resultado.
void VentasWindow::cobrar()
{
    if (controlador_.carritoVacio())
        return;

    const double efectivo = spinEfectivo->value();
    std::string error;
    if (!controlador_.cobrar(efectivo, &error))
    {
        QMessageBox::warning(this, "No se pudo cobrar", QString::fromStdString(error));
        return;
    }

    const VentasCobro venta = controlador_.ventaCobrada();
    lblCambio->setText("Cambio: " + Formato::colones(venta.cambio()));
    spinEfectivo->setValue(0.0);
    actualizarVista();
    txtCodigo->setFocus();

    // [VENTAS] El controlador ya desconto el stock y registro la venta.
    QMessageBox::information(this, "Venta registrada",
        "Venta N.º " + Formato::numero(venta.numero()) + " cobrada y guardada en el historial.\n"
        "Total: " + Formato::colones(venta.total()) + "\n"
        "Cambio: " + Formato::colones(venta.cambio()));
}
