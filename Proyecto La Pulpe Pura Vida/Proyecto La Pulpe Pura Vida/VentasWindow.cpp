#include "VentasWindow.h"

#include <utility>

#include <QColor>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVariant>
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

    QHBoxLayout* crearFilaResumen(QWidget* padre, const QString& etiqueta, QLabel*& valor)
    {
        QHBoxLayout* fila = new QHBoxLayout();
        fila->setContentsMargins(0, 0, 0, 0);

        QLabel* lblEtiqueta = new QLabel(etiqueta, padre);
        lblEtiqueta->setObjectName("lblResumenClave");

        valor = new QLabel("—", padre);
        valor->setObjectName("lblResumenValor");
        valor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        fila->addWidget(lblEtiqueta);
        fila->addStretch();
        fila->addWidget(valor);
        return fila;
    }

    QString hojaEstilosDialogo()
    {
        return QStringLiteral(
            "QDialog { background-color: #fff8ee; }"
            "QLabel { color: #3b2a1a; font-family: 'Segoe UI'; background: transparent; }"
            "QLabel#dlgTitulo { font-size: 20px; font-weight: 900; }"
            "QLabel#dlgTexto { font-size: 13px; color: #6b5a48; }"
            "QLabel#dlgEtiqueta { font-size: 11px; font-weight: bold; color: #8a6a4f; }"
            "QLabel#dlgValor { font-size: 15px; font-weight: bold; color: #3b2a1a; }"
            "QLabel#dlgTotal { font-size: 30px; font-weight: 900; color: #e67e22; }"
            "QFrame#dlgTarjeta { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
            "QPushButton#dlgPrimario { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 10px 22px; font-weight: bold; }"
            "QPushButton#dlgPrimario:hover { background-color: #cf6d17; }"
            "QPushButton#dlgSecundario { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 10px 18px; font-weight: bold; }"
            "QPushButton#dlgSecundario:hover { border: 1px solid #e67e22; }"
        );
    }

    bool mostrarDialogo(QWidget* padre, const QString& titulo, const QString& texto,
        bool esError, bool preguntar)
    {
        QDialog dialogo(padre);
        dialogo.setWindowTitle(titulo);
        dialogo.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        dialogo.setModal(true);
        dialogo.setMinimumWidth(420);
        dialogo.setStyleSheet(hojaEstilosDialogo());

        QVBoxLayout* layout = new QVBoxLayout(&dialogo);
        layout->setContentsMargins(24, 22, 24, 20);
        layout->setSpacing(12);

        QLabel* lblTitulo = new QLabel(titulo, &dialogo);
        lblTitulo->setObjectName("dlgTitulo");
        if (esError)
            lblTitulo->setStyleSheet("color: #c0392b;");

        QLabel* lblTexto = new QLabel(texto, &dialogo);
        lblTexto->setObjectName("dlgTexto");
        lblTexto->setWordWrap(true);
        lblTexto->setTextFormat(Qt::PlainText);

        layout->addWidget(lblTitulo);
        layout->addWidget(lblTexto);

        QHBoxLayout* layoutBotones = new QHBoxLayout();
        layoutBotones->addStretch();

        QPushButton* btnAceptar = new QPushButton(preguntar ? "Si, continuar" : "Aceptar", &dialogo);
        btnAceptar->setObjectName("dlgPrimario");
        btnAceptar->setCursor(Qt::PointingHandCursor);
        btnAceptar->setDefault(true);

        if (preguntar)
        {
            QPushButton* btnCancelar = new QPushButton("Cancelar", &dialogo);
            btnCancelar->setObjectName("dlgSecundario");
            btnCancelar->setCursor(Qt::PointingHandCursor);
            QObject::connect(btnCancelar, &QPushButton::clicked, &dialogo, &QDialog::reject);
            layoutBotones->addWidget(btnCancelar);
        }

        layoutBotones->addWidget(btnAceptar);
        layout->addLayout(layoutBotones);

        QObject::connect(btnAceptar, &QPushButton::clicked, &dialogo, &QDialog::accept);
        return dialogo.exec() == QDialog::Accepted;
    }

    void mostrarVentaRegistrada(QWidget* padre, const VentasCobro& venta)
    {
        QDialog dialogo(padre);
        dialogo.setWindowTitle("Venta registrada");
        dialogo.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        dialogo.setModal(true);
        dialogo.setMinimumWidth(420);
        dialogo.setStyleSheet(hojaEstilosDialogo());

        QVBoxLayout* layout = new QVBoxLayout(&dialogo);
        layout->setContentsMargins(24, 22, 24, 20);
        layout->setSpacing(12);

        QLabel* lblTitulo = new QLabel("Venta registrada", &dialogo);
        lblTitulo->setObjectName("dlgTitulo");
        lblTitulo->setStyleSheet("color: #27ae60;");

        QLabel* lblTexto = new QLabel(
            "La venta N.º " + Formato::numero(venta.numero()) +
            " se cobro y se guardo correctamente en el historial.", &dialogo);
        lblTexto->setObjectName("dlgTexto");
        lblTexto->setWordWrap(true);
        lblTexto->setTextFormat(Qt::PlainText);

        QFrame* tarjeta = new QFrame(&dialogo);
        tarjeta->setObjectName("dlgTarjeta");
        QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
        layoutTarjeta->setContentsMargins(18, 14, 18, 14);
        layoutTarjeta->setSpacing(8);

        QLabel* lblEtiquetaTotal = new QLabel("Total cobrado", tarjeta);
        lblEtiquetaTotal->setObjectName("dlgEtiqueta");

        QLabel* lblTotal = new QLabel(Formato::colones(venta.total()), tarjeta);
        lblTotal->setObjectName("dlgTotal");

        layoutTarjeta->addWidget(lblEtiquetaTotal);
        layoutTarjeta->addWidget(lblTotal);

        QHBoxLayout* filaEfectivo = new QHBoxLayout();
        QLabel* lblEfectivoEtiqueta = new QLabel("Efectivo recibido", tarjeta);
        lblEfectivoEtiqueta->setObjectName("dlgEtiqueta");
        QLabel* lblEfectivoValor = new QLabel(Formato::colones(venta.pago()), tarjeta);
        lblEfectivoValor->setObjectName("dlgValor");
        filaEfectivo->addWidget(lblEfectivoEtiqueta);
        filaEfectivo->addStretch();
        filaEfectivo->addWidget(lblEfectivoValor);

        QHBoxLayout* filaCambio = new QHBoxLayout();
        QLabel* lblCambioEtiqueta = new QLabel("Cambio", tarjeta);
        lblCambioEtiqueta->setObjectName("dlgEtiqueta");
        QLabel* lblCambioValor = new QLabel(Formato::colones(venta.cambio()), tarjeta);
        lblCambioValor->setObjectName("dlgValor");
        lblCambioValor->setStyleSheet("color: #27ae60;");
        filaCambio->addWidget(lblCambioEtiqueta);
        filaCambio->addStretch();
        filaCambio->addWidget(lblCambioValor);

        layoutTarjeta->addLayout(filaEfectivo);
        layoutTarjeta->addLayout(filaCambio);

        QHBoxLayout* layoutBotones = new QHBoxLayout();
        layoutBotones->addStretch();

        QPushButton* btnAceptar = new QPushButton("Aceptar", &dialogo);
        btnAceptar->setObjectName("dlgPrimario");
        btnAceptar->setCursor(Qt::PointingHandCursor);
        btnAceptar->setDefault(true);
        QObject::connect(btnAceptar, &QPushButton::clicked, &dialogo, &QDialog::accept);
        layoutBotones->addWidget(btnAceptar);

        layout->addWidget(lblTitulo);
        layout->addWidget(lblTexto);
        layout->addWidget(tarjeta);
        layout->addLayout(layoutBotones);

        dialogo.exec();
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
    connect(txtBuscar, &QLineEdit::returnPressed, this, &VentasWindow::agregarAlCarrito);
    connect(txtBuscar, &QLineEdit::textChanged, this, &VentasWindow::actualizarResultadosBusqueda);
    connect(listaResultados, &QListWidget::itemDoubleClicked, this, &VentasWindow::agregarResultadoSeleccionado);
    connect(btnVaciar, &QPushButton::clicked, this, &VentasWindow::vaciarCarrito);
    connect(btnCobrar, &QPushButton::clicked, this, &VentasWindow::cobrar);

    actualizarVista();
    actualizarResultadosBusqueda();
    txtBuscar->setFocus();
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
    layout->setContentsMargins(28, 20, 28, 20);
    layout->setSpacing(14);

    // Barra superior: volver + titulo + cajero.
    QFrame* barraSuperior = new QFrame(panel);
    barraSuperior->setObjectName("barraSuperior");
    QHBoxLayout* layoutEncabezado = new QHBoxLayout(barraSuperior);
    layoutEncabezado->setContentsMargins(16, 12, 16, 12);
    layoutEncabezado->setSpacing(14);

    btnVolverMenu = new QPushButton("← Menú Principal", barraSuperior);
    btnVolverMenu->setObjectName("btnVolver");
    btnVolverMenu->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* layoutTitulos = new QVBoxLayout();
    layoutTitulos->setContentsMargins(0, 0, 0, 0);
    layoutTitulos->setSpacing(0);

    QLabel* lblTitulo = new QLabel("Ventas y Cobro", barraSuperior);
    lblTitulo->setObjectName("lblTituloPantalla");

    QLabel* lblSubtitulo = new QLabel("Punto de venta · Caja 01", barraSuperior);
    lblSubtitulo->setObjectName("lblSubtituloPantalla");

    layoutTitulos->addWidget(lblTitulo);
    layoutTitulos->addWidget(lblSubtitulo);

    QLabel* lblCajero = new QLabel("Cajero: " + QString::fromStdString(cajeroActual()), barraSuperior);
    lblCajero->setObjectName("lblTag");

    layoutEncabezado->addWidget(btnVolverMenu);
    layoutEncabezado->addLayout(layoutTitulos);
    layoutEncabezado->addStretch();
    layoutEncabezado->addWidget(lblCajero);

    layout->addWidget(barraSuperior);

    // Cuerpo: busqueda y carrito a la izquierda, resumen y cobro a la derecha.
    QHBoxLayout* layoutCuerpo = new QHBoxLayout();
    layoutCuerpo->setSpacing(16);

    QVBoxLayout* layoutIzquierda = new QVBoxLayout();
    layoutIzquierda->setSpacing(12);

    // Tarjeta de busqueda de productos.
    QFrame* tarjetaBusqueda = new QFrame(panel);
    tarjetaBusqueda->setObjectName("tarjetaPanel");
    QVBoxLayout* layoutBusqueda = new QVBoxLayout(tarjetaBusqueda);
    layoutBusqueda->setContentsMargins(16, 12, 16, 12);
    layoutBusqueda->setSpacing(8);

    QLabel* lblBuscar = new QLabel("Buscar producto", tarjetaBusqueda);
    lblBuscar->setObjectName("lblSeccion");

    QLabel* lblAyuda = new QLabel(
        "Escriba el codigo o el nombre y presione Enter. Doble clic en un resultado para agregarlo.",
        tarjetaBusqueda);
    lblAyuda->setObjectName("lblAyuda");
    lblAyuda->setWordWrap(true);

    QHBoxLayout* layoutFilaBusqueda = new QHBoxLayout();
    layoutFilaBusqueda->setSpacing(8);

    txtBuscar = new QLineEdit(tarjetaBusqueda);
    txtBuscar->setPlaceholderText("Codigo o nombre del producto...");
    txtBuscar->setClearButtonEnabled(true);
    txtBuscar->setMinimumHeight(38);

    btnAgregar = new QPushButton("Agregar", tarjetaBusqueda);
    btnAgregar->setObjectName("btnPrimario");
    btnAgregar->setCursor(Qt::PointingHandCursor);
    btnAgregar->setMinimumHeight(38);

    layoutFilaBusqueda->addWidget(txtBuscar, 1);
    layoutFilaBusqueda->addWidget(btnAgregar);

    listaResultados = new QListWidget(tarjetaBusqueda);
    listaResultados->setObjectName("listaResultados");
    listaResultados->setMaximumHeight(150);
    listaResultados->setSelectionMode(QAbstractItemView::SingleSelection);
    listaResultados->setCursor(Qt::PointingHandCursor);

    layoutBusqueda->addWidget(lblBuscar);
    layoutBusqueda->addWidget(lblAyuda);
    layoutBusqueda->addLayout(layoutFilaBusqueda);
    layoutBusqueda->addWidget(listaResultados);

    layoutIzquierda->addWidget(tarjetaBusqueda);

    // Tarjeta del carrito.
    QFrame* tarjetaCarrito = new QFrame(panel);
    tarjetaCarrito->setObjectName("tarjetaPanel");
    QVBoxLayout* layoutCarrito = new QVBoxLayout(tarjetaCarrito);
    layoutCarrito->setContentsMargins(16, 12, 16, 12);
    layoutCarrito->setSpacing(10);

    QHBoxLayout* layoutTituloCarrito = new QHBoxLayout();
    QLabel* lblCarrito = new QLabel("Carrito de la venta", tarjetaCarrito);
    lblCarrito->setObjectName("lblSeccion");

    btnVaciar = new QPushButton("Vaciar", tarjetaCarrito);
    btnVaciar->setObjectName("btnSuave");
    btnVaciar->setCursor(Qt::PointingHandCursor);

    layoutTituloCarrito->addWidget(lblCarrito);
    layoutTituloCarrito->addStretch();
    layoutTituloCarrito->addWidget(btnVaciar);

    tablaCarrito = new QTableWidget(0, 6, tarjetaCarrito);
    tablaCarrito->setHorizontalHeaderLabels({ "Código", "Producto", "Cant.", "Precio", "Subtotal", "Acciones" });
    tablaCarrito->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tablaCarrito->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tablaCarrito->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tablaCarrito->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tablaCarrito->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tablaCarrito->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    tablaCarrito->setColumnWidth(5, 118);
    tablaCarrito->verticalHeader()->setVisible(false);
    tablaCarrito->verticalHeader()->setDefaultSectionSize(40);
    tablaCarrito->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaCarrito->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaCarrito->setAlternatingRowColors(true);
    tablaCarrito->setShowGrid(false);

    QLabel* lblAcciones = new QLabel(
        "Use los botones +, − y × de cada linea para modificar el carrito.", tarjetaCarrito);
    lblAcciones->setObjectName("lblAyuda");

    layoutCarrito->addLayout(layoutTituloCarrito);
    layoutCarrito->addWidget(tablaCarrito, 1);
    layoutCarrito->addWidget(lblAcciones);

    layoutIzquierda->addWidget(tarjetaCarrito, 1);

    layoutCuerpo->addLayout(layoutIzquierda, 1);

    // Columna derecha: resumen de la venta y pago.
    QVBoxLayout* layoutDerecha = new QVBoxLayout();
    layoutDerecha->setSpacing(12);

    QFrame* tarjetaResumen = new QFrame(panel);
    tarjetaResumen->setObjectName("tarjetaResumen");
    QVBoxLayout* layoutResumen = new QVBoxLayout(tarjetaResumen);
    layoutResumen->setContentsMargins(18, 14, 18, 14);
    layoutResumen->setSpacing(10);

    QLabel* lblResumenTitulo = new QLabel("Resumen de la venta", tarjetaResumen);
    lblResumenTitulo->setObjectName("lblSeccion");

    layoutResumen->addWidget(lblResumenTitulo);
    layoutResumen->addLayout(crearFilaResumen(tarjetaResumen, "Subtotal", lblSubtotal));
    layoutResumen->addLayout(crearFilaResumen(tarjetaResumen, "IVA (13%)", lblIva));

    QFrame* lineaDivisoria = new QFrame(tarjetaResumen);
    lineaDivisoria->setObjectName("lineaDivisoria");
    lineaDivisoria->setFrameShape(QFrame::HLine);
    layoutResumen->addWidget(lineaDivisoria);

    QLabel* lblTotalEtiqueta = new QLabel("Total a pagar", tarjetaResumen);
    lblTotalEtiqueta->setObjectName("lblTotalEtiqueta");

    lblTotal = new QLabel("—", tarjetaResumen);
    lblTotal->setObjectName("lblTotalGrande");
    lblTotal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layoutResumen->addWidget(lblTotalEtiqueta);
    layoutResumen->addWidget(lblTotal);

    layoutDerecha->addWidget(tarjetaResumen);

    QFrame* tarjetaCobro = new QFrame(panel);
    tarjetaCobro->setObjectName("tarjetaCobro");
    QVBoxLayout* layoutTarjetaCobro = new QVBoxLayout(tarjetaCobro);
    layoutTarjetaCobro->setContentsMargins(18, 14, 18, 14);
    layoutTarjetaCobro->setSpacing(8);

    QLabel* lblPagoTitulo = new QLabel("Pago en efectivo", tarjetaCobro);
    lblPagoTitulo->setObjectName("lblSeccion");

    QLabel* lblEfectivo = new QLabel("Efectivo recibido", tarjetaCobro);
    lblEfectivo->setObjectName("lblResumenClave");

    spinEfectivo = new QDoubleSpinBox(tarjetaCobro);
    spinEfectivo->setPrefix("₡ ");
    spinEfectivo->setDecimals(0);
    spinEfectivo->setSingleStep(100);
    spinEfectivo->setMaximum(10000000);
    spinEfectivo->setAlignment(Qt::AlignRight);
    spinEfectivo->setMinimumHeight(40);

    lblCambio = new QLabel("Cambio: —", tarjetaCobro);
    lblCambio->setObjectName("lblCambio");
    lblCambio->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    btnCobrar = new QPushButton("Cobrar venta", tarjetaCobro);
    btnCobrar->setObjectName("btnCobrar");
    btnCobrar->setCursor(Qt::PointingHandCursor);
    btnCobrar->setMinimumHeight(48);

    layoutTarjetaCobro->addWidget(lblPagoTitulo);
    layoutTarjetaCobro->addWidget(lblEfectivo);
    layoutTarjetaCobro->addWidget(spinEfectivo);
    layoutTarjetaCobro->addWidget(lblCambio);
    layoutTarjetaCobro->addWidget(btnCobrar);

    layoutDerecha->addWidget(tarjetaCobro);

    QLabel* lblNota = new QLabel(
        "El total incluye el IVA. Al cobrar se descuenta el inventario y se registra la venta.",
        panel);
    lblNota->setObjectName("lblAyuda");
    lblNota->setWordWrap(true);
    layoutDerecha->addWidget(lblNota);
    layoutDerecha->addStretch();

    QWidget* contenedorDerecha = new QWidget(panel);
    contenedorDerecha->setLayout(layoutDerecha);
    contenedorDerecha->setFixedWidth(340);

    layoutCuerpo->addWidget(contenedorDerecha);

    layout->addLayout(layoutCuerpo, 1);

    return panel;
}

void VentasWindow::aplicarEstilos()
{
    setStyleSheet(
        "VentasWindow { background-color: #fff8ee; }"
        "QWidget { font-family: 'Segoe UI'; font-size: 14px; color: #3b2a1a; }"
        "QLabel { background: transparent; }"
        "QLabel#lblTituloPantalla { font-size: 24px; font-weight: 900; }"
        "QLabel#lblSubtituloPantalla { font-size: 12px; color: #8a6a4f; }"
        "QLabel#lblTag { background-color: #fff3e0; color: #8a6a4f; border: 1px solid #f0d9b5; border-radius: 10px; padding: 6px 14px; font-size: 11px; font-weight: bold; }"
        "QLabel#lblSeccion { font-size: 12px; font-weight: 900; color: #8a6a4f; }"
        "QLabel#lblAyuda { font-size: 11px; color: #a08a72; }"
        "QLabel#lblResumenClave { font-size: 13px; color: #7a6650; }"
        "QLabel#lblResumenValor { font-size: 15px; font-weight: bold; color: #3b2a1a; }"
        "QLabel#lblTotalEtiqueta { font-size: 11px; font-weight: bold; color: #8a6a4f; }"
        "QLabel#lblTotalGrande { font-size: 32px; font-weight: 900; color: #e67e22; }"
        "QFrame#barraSuperior { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QFrame#tarjetaPanel { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QFrame#tarjetaResumen { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QFrame#tarjetaCobro { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }"
        "QFrame#lineaDivisoria { background-color: #f0d9b5; border: none; max-height: 1px; }"
        "QPushButton#btnVolver { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }"
        "QPushButton#btnVolver:hover { background-color: #fffaf3; border: 1px solid #e67e22; }"
        "QPushButton#btnPrimario { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 9px 18px; font-weight: bold; }"
        "QPushButton#btnPrimario:hover { background-color: #cf6d17; }"
        "QPushButton#btnSuave { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 8px 14px; font-weight: bold; }"
        "QPushButton#btnSuave:hover { background-color: #fffaf3; border: 1px solid #e67e22; }"
        "QPushButton#btnSuave:disabled { color: #c8bbaa; border: 1px solid #efe2cf; }"
        "QPushButton#btnAccionFila { background-color: #fff3e0; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 6px; font-size: 16px; font-weight: 900; padding: 0px; }"
        "QPushButton#btnAccionFila:hover { background-color: #ffe3c2; border: 1px solid #e67e22; color: #3b2a1a; }"
        "QPushButton#btnAccionFila:pressed { background-color: #f5d3ae; }"
        "QPushButton#btnAccionEliminar { background-color: #fdecea; color: #c0392b; border: 1px solid #f5c6c0; border-radius: 6px; font-size: 16px; font-weight: 900; padding: 0px; }"
        "QPushButton#btnAccionEliminar:hover { background-color: #f9d7d2; border: 1px solid #c0392b; }"
        "QPushButton#btnAccionEliminar:pressed { background-color: #f3c1ba; }"
        "QPushButton#btnCobrar { background-color: #27ae60; color: #ffffff; border: none; border-radius: 8px; padding: 11px 22px; font-size: 15px; font-weight: bold; }"
        "QPushButton#btnCobrar:hover { background-color: #1e8449; }"
        "QPushButton#btnCobrar:disabled { background-color: #d5cdbf; color: #ffffff; }"
        "QLineEdit, QDoubleSpinBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 8px 10px; }"
        "QLineEdit:focus, QDoubleSpinBox:focus { border: 1px solid #e67e22; }"
        "QListWidget#listaResultados { background-color: #fffdf8; border: 1px solid #f0d9b5; border-radius: 8px; padding: 4px; }"
        "QListWidget#listaResultados::item { padding: 7px 8px; border-radius: 6px; color: #4a3a2a; }"
        "QListWidget#listaResultados::item:selected { background-color: #ffe3c2; color: #3b2a1a; }"
        "QListWidget#listaResultados::item:hover { background-color: #fff3e0; }"
        "QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 10px; alternate-background-color: #fffaf3; selection-background-color: #ffe3c2; selection-color: #3b2a1a; }"
        "QTableWidget::item { padding: 4px 6px; }"
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
        tablaCarrito->setCellWidget(i, 5, crearAccionesFila(linea.codigo().toStdString()));
    }

    lblSubtotal->setText(Formato::colones(controlador_.subtotal()));
    lblIva->setText(Formato::colones(controlador_.iva()));
    lblTotal->setText(Formato::colones(controlador_.total()));

    const bool hayLineas = !controlador_.carritoVacio();
    btnCobrar->setEnabled(hayLineas);
    btnVaciar->setEnabled(hayLineas);
}

// [VENTAS] Llena la lista de resultados con los productos activos que
// coinciden con el texto buscado (codigo o nombre).
void VentasWindow::actualizarResultadosBusqueda()
{
    const QString filtro = txtBuscar->text().trimmed();
    listaResultados->clear();

    const std::vector<Producto> productos = inventario_->getProductosActivos();
    int mostrados = 0;

    for (const Producto& producto : productos)
    {
        const QString codigo = QString::fromStdString(producto.getCodigo());
        const QString nombre = QString::fromStdString(producto.getNombre());

        if (!filtro.isEmpty()
            && !codigo.contains(filtro, Qt::CaseInsensitive)
            && !nombre.contains(filtro, Qt::CaseInsensitive))
            continue;

        QString detalle = codigo + "  ·  " + nombre + "  ·  " + Formato::colones(producto.getPrecio());
        if (producto.getStock() > 0)
            detalle += "  ·  " + QString::number(producto.getStock()) + " disp.";
        else
            detalle += "  ·  Agotado";

        QListWidgetItem* item = new QListWidgetItem(detalle, listaResultados);
        item->setData(Qt::UserRole, codigo);

        if (producto.getStock() <= 0)
        {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setForeground(QColor("#b8a998"));
        }

        if (++mostrados >= 12)
            break;
    }

    if (listaResultados->count() > 0)
        listaResultados->setCurrentRow(0);
}

// ---------------- Acciones del carrito ----------------

// [VENTAS] Agrega el producto escrito o el resultado seleccionado.
void VentasWindow::agregarAlCarrito()
{
    const QString texto = txtBuscar->text().trimmed();

    if (!texto.isEmpty())
    {
        Producto* producto = inventario_->buscarPorCodigo(texto.toStdString());
        if (producto != nullptr)
        {
            agregarProducto(producto->getCodigo());
            return;
        }

        const std::string codigo = codigoResultadoSeleccionado();
        if (!codigo.empty())
        {
            agregarProducto(codigo);
            return;
        }
    }

    if (texto.isEmpty())
    {
        mostrarDialogo(this, "Buscar producto",
            "Escriba el codigo o el nombre del producto, o seleccione un resultado de la lista.",
            false, false);
        txtBuscar->setFocus();
        return;
    }

    mostrarDialogo(this, "Producto no encontrado",
        "No hay productos que coincidan con \"" + texto + "\". Verifique el codigo o el nombre.",
        true, false);
    txtBuscar->selectAll();
    txtBuscar->setFocus();
}

// [VENTAS] Agrega el producto resaltado en la lista de resultados.
void VentasWindow::agregarResultadoSeleccionado()
{
    const std::string codigo = codigoResultadoSeleccionado();
    if (!codigo.empty())
        agregarProducto(codigo);
}

// [VENTAS] Valida stock y agrega una unidad del producto al carrito.
void VentasWindow::agregarProducto(const std::string& codigo)
{
    Producto* producto = inventario_->buscarPorCodigo(codigo);
    if (producto == nullptr)
    {
        mostrarDialogo(this, "Producto no encontrado",
            "No existe un producto con ese codigo.", true, false);
        return;
    }

    if (!controlador_.agregarProducto(codigo))
    {
        mostrarDialogo(this, "Stock insuficiente",
            "No hay mas unidades disponibles de \"" +
            QString::fromStdString(producto->getNombre()) + "\".", true, false);
        return;
    }

    actualizarVista();
    actualizarResultadosBusqueda();
    txtBuscar->setFocus();
    txtBuscar->selectAll();
}

// [VENTAS] Obtiene el codigo del resultado seleccionado en la busqueda.
std::string VentasWindow::codigoResultadoSeleccionado() const
{
    const QListWidgetItem* item = listaResultados->currentItem();
    if (item == nullptr || !(item->flags() & Qt::ItemIsEnabled))
        return std::string();
    return item->data(Qt::UserRole).toString().toStdString();
}

// [VENTAS] Crea los botones +, − y × que acompanan a cada linea del carrito.
QWidget* VentasWindow::crearAccionesFila(const std::string& codigo)
{
    QWidget* contenedor = new QWidget(tablaCarrito);
    QHBoxLayout* layout = new QHBoxLayout(contenedor);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->setAlignment(Qt::AlignCenter);

    QPushButton* btnMenos = new QPushButton("−", contenedor);
    btnMenos->setObjectName("btnAccionFila");
    btnMenos->setToolTip("Quitar una unidad");
    btnMenos->setCursor(Qt::PointingHandCursor);
    btnMenos->setFixedSize(30, 28);

    QPushButton* btnMas = new QPushButton("+", contenedor);
    btnMas->setObjectName("btnAccionFila");
    btnMas->setToolTip("Agregar una unidad");
    btnMas->setCursor(Qt::PointingHandCursor);
    btnMas->setFixedSize(30, 28);

    QPushButton* btnEliminar = new QPushButton("×", contenedor);
    btnEliminar->setObjectName("btnAccionEliminar");
    btnEliminar->setToolTip("Quitar el producto del carrito");
    btnEliminar->setCursor(Qt::PointingHandCursor);
    btnEliminar->setFixedSize(30, 28);

    layout->addWidget(btnMenos);
    layout->addWidget(btnMas);
    layout->addWidget(btnEliminar);

    QObject::connect(btnMenos, &QPushButton::clicked, this, [this, codigo]() {
        quitarUnidadDe(codigo);
    });
    QObject::connect(btnMas, &QPushButton::clicked, this, [this, codigo]() {
        agregarUnidad(codigo);
    });
    QObject::connect(btnEliminar, &QPushButton::clicked, this, [this, codigo]() {
        quitarProductoDe(codigo);
    });

    return contenedor;
}

// [VENTAS] Agrega una unidad del producto de una linea del carrito.
void VentasWindow::agregarUnidad(const std::string& codigo)
{
    Producto* producto = inventario_->buscarPorCodigo(codigo);
    if (!controlador_.agregarProducto(codigo))
    {
        const QString nombre = producto != nullptr
            ? QString::fromStdString(producto->getNombre())
            : QString::fromStdString(codigo);
        mostrarDialogo(this, "Stock insuficiente",
            "No hay mas unidades disponibles de \"" + nombre + "\".", true, false);
        return;
    }

    // Se refresca despues de que el boton termine de procesar el clic.
    QTimer::singleShot(0, this, &VentasWindow::actualizarVista);
}

// [VENTAS] Quita una unidad del producto de una linea del carrito.
void VentasWindow::quitarUnidadDe(const std::string& codigo)
{
    controlador_.quitarUnidad(codigo);
    QTimer::singleShot(0, this, &VentasWindow::actualizarVista);
}

// [VENTAS] Quita la linea completa del carrito.
void VentasWindow::quitarProductoDe(const std::string& codigo)
{
    controlador_.quitarProducto(codigo);
    QTimer::singleShot(0, this, &VentasWindow::actualizarVista);
}

// [VENTAS] Pide confirmacion y vacia todo el carrito.
void VentasWindow::vaciarCarrito()
{
    if (controlador_.carritoVacio())
        return;

    if (mostrarDialogo(this, "Vaciar carrito", "¿Seguro que desea vaciar el carrito?", false, true))
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
        mostrarDialogo(this, "No se pudo cobrar", QString::fromStdString(error), true, false);
        return;
    }

    const VentasCobro venta = controlador_.ventaCobrada();
    lblCambio->setText("Cambio: " + Formato::colones(venta.cambio()));
    spinEfectivo->setValue(0.0);
    actualizarVista();
    actualizarResultadosBusqueda();
    txtBuscar->setFocus();

    // [VENTAS] El controlador ya desconto el stock y registro la venta.
    mostrarVentaRegistrada(this, venta);
}
