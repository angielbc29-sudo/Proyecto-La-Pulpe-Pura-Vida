#include "VentanaHistorial.h"

#include <utility>
#include <vector>

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "Errores.h"
#include "Formato.h"
#include "Historial.h"
#include "Linea.h"
#include "TicketHtml.h"
#include "TicketTxt.h"

namespace
{
    QTableWidgetItem* crearItem(const QString& texto, bool centrado)
    {
        QTableWidgetItem* item = new QTableWidgetItem(texto);
        item->setFlags(Qt::ItemIsEnabled);
        if (centrado)
            item->setTextAlignment(Qt::AlignCenter);
        return item;
    }

    QFrame* crearTarjetaMetrica(QWidget* padre, const QString& etiqueta,
        QLabel*& valor, const QString& detalle,
        const QString& color)
    {
        QFrame* tarjeta = new QFrame(padre);
        tarjeta->setObjectName("tarjetaMetrica");

        QVBoxLayout* layout = new QVBoxLayout(tarjeta);
        layout->setContentsMargins(18, 14, 18, 14);
        layout->setSpacing(2);

        QLabel* lblEtiqueta = new QLabel(etiqueta, tarjeta);
        lblEtiqueta->setObjectName("lblEtiquetaMetrica");

        valor = new QLabel("—", tarjeta);
        valor->setObjectName("lblValorMetrica");
        valor->setStyleSheet("font-size: 26px; font-weight: 900; color: " + color + ";");

        QLabel* lblDetalle = new QLabel(detalle, tarjeta);
        lblDetalle->setObjectName("lblDetalleMetrica");

        layout->addWidget(lblEtiqueta);
        layout->addWidget(valor);
        layout->addWidget(lblDetalle);
        return tarjeta;
    }
}

VentanaHistorial::VentanaHistorial(std::shared_ptr<Historial> historial, QWidget* parent)
    : QWidget(parent), historial_(std::move(historial))
{
    setWindowTitle("La Pulpe Pura Vida - Comprobantes e Historial");
    setAttribute(Qt::WA_StyledBackground, true);

    paginas = new QStackedWidget(this);

    construirPaginaHistorial();
    construirPaginaDetalle();
    construirPaginaError();

    paginas->addWidget(paginaHistorial);
    paginas->addWidget(paginaDetalle);
    paginas->addWidget(paginaError);
    paginas->setCurrentWidget(paginaHistorial);

    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->addWidget(paginas);
    setLayout(layoutPrincipal);

    aplicarEstilos();

    connect(btnVolverMenu, &QPushButton::clicked, this, &VentanaHistorial::volverAlMenu);
    connect(btnVolverError, &QPushButton::clicked, this, &VentanaHistorial::volverAlMenu);
    connect(btnReintentarCarga, &QPushButton::clicked, this, &VentanaHistorial::cargarHistorial);
    connect(btnFiltrar, &QPushButton::clicked, this, &VentanaHistorial::aplicarFiltro);
    connect(btnLimpiar, &QPushButton::clicked, this, &VentanaHistorial::limpiarFiltros);
    connect(txtBuscar, &QLineEdit::returnPressed, this, &VentanaHistorial::aplicarFiltro);

    connect(btnVolverHistorial, &QPushButton::clicked, this, [this]()
        {
            paginas->setCurrentWidget(paginaHistorial);
        });

    connect(cmbFormato, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int) { actualizarVistaPrevia(); });

    connect(btnImprimir, &QPushButton::clicked, this, &VentanaHistorial::imprimirTicket);
    connect(btnReintentarImpresion, &QPushButton::clicked, this, &VentanaHistorial::imprimirTicket);
    connect(btnGuardar, &QPushButton::clicked, this, &VentanaHistorial::guardarCopia);

    cargarHistorial();
}

// ---------------- Pantalla del historial ----------------

void VentanaHistorial::construirPaginaHistorial()
{
    paginaHistorial = new QWidget(this);

    btnVolverMenu = new QPushButton("← Menú Principal", paginaHistorial);
    btnVolverMenu->setObjectName("btnVolver");
    btnVolverMenu->setCursor(Qt::PointingHandCursor);

    QLabel* lblTitulo = new QLabel("Historial de Ventas", paginaHistorial);
    lblTitulo->setObjectName("lblTituloPantalla");

    QLabel* lblSubtitulo = new QLabel(
        "Consulta las ventas guardadas y vuelve a imprimir su ticket / factura.",
        paginaHistorial);
    lblSubtitulo->setObjectName("lblSubtituloPantalla");

    QLabel* lblTag = new QLabel("HISTORIAL · Permanente · solo lectura", paginaHistorial);
    lblTag->setObjectName("lblTag");

    QVBoxLayout* layoutTitulos = new QVBoxLayout();
    layoutTitulos->setSpacing(2);
    layoutTitulos->addWidget(lblTitulo);
    layoutTitulos->addWidget(lblSubtitulo);

    QHBoxLayout* layoutBarra = new QHBoxLayout();
    layoutBarra->setSpacing(16);
    layoutBarra->addWidget(btnVolverMenu);
    layoutBarra->addLayout(layoutTitulos, 1);
    layoutBarra->addWidget(lblTag, 0, Qt::AlignTop);

    QHBoxLayout* layoutMetricas = new QHBoxLayout();
    layoutMetricas->setSpacing(14);
    layoutMetricas->addWidget(crearTarjetaMetrica(paginaHistorial,
        "VENTAS GUARDADAS", lblVentasGuardadas, "en el historial", "#3b2a1a"));
    layoutMetricas->addWidget(crearTarjetaMetrica(paginaHistorial,
        "TOTAL VENDIDO", lblTotalVendido, "suma de todas las ventas", "#1e8449"));
    layoutMetricas->addWidget(crearTarjetaMetrica(paginaHistorial,
        "ÚLTIMA VENTA", lblUltimaVenta, "fecha y hora", "#e67e22"));

    txtBuscar = new QLineEdit(paginaHistorial);
    txtBuscar->setPlaceholderText("Buscar por N.º de venta o cajero...");
    txtBuscar->setClearButtonEnabled(true);

    fechaDesde = new QDateEdit(paginaHistorial);
    fechaDesde->setCalendarPopup(true);
    fechaDesde->setDisplayFormat("dd/MM/yyyy");

    fechaHasta = new QDateEdit(paginaHistorial);
    fechaHasta->setCalendarPopup(true);
    fechaHasta->setDisplayFormat("dd/MM/yyyy");

    btnFiltrar = new QPushButton("Filtrar", paginaHistorial);
    btnFiltrar->setObjectName("btnFiltrar");
    btnFiltrar->setCursor(Qt::PointingHandCursor);

    btnLimpiar = new QPushButton("Limpiar", paginaHistorial);
    btnLimpiar->setObjectName("btnLimpiar");
    btnLimpiar->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutFiltros = new QHBoxLayout();
    layoutFiltros->setSpacing(10);
    layoutFiltros->addWidget(txtBuscar, 2);
    layoutFiltros->addWidget(new QLabel("Desde:", paginaHistorial));
    layoutFiltros->addWidget(fechaDesde);
    layoutFiltros->addWidget(new QLabel("Hasta:", paginaHistorial));
    layoutFiltros->addWidget(fechaHasta);
    layoutFiltros->addWidget(btnFiltrar);
    layoutFiltros->addWidget(btnLimpiar);

    tabla = new QTableWidget(paginaHistorial);
    tabla->setColumnCount(7);
    tabla->setHorizontalHeaderLabels({ "N.º", "Fecha y hora", "Cajero", "Artículos",
                                       "Total", "Cambio", "Acción" });
    tabla->verticalHeader()->setVisible(false);
    tabla->verticalHeader()->setDefaultSectionSize(52);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setSelectionMode(QAbstractItemView::NoSelection);
    tabla->setFocusPolicy(Qt::NoFocus);
    tabla->setShowGrid(false);

    QHeaderView* cabecera = tabla->horizontalHeader();
    cabecera->setSectionResizeMode(QHeaderView::ResizeToContents);
    cabecera->setSectionResizeMode(2, QHeaderView::Stretch);
    cabecera->setSectionResizeMode(6, QHeaderView::Fixed);
    tabla->setColumnWidth(6, 170);
    cabecera->setHighlightSections(false);

    lblSinVentas = new QLabel("No hay ventas para mostrar.", paginaHistorial);
    lblSinVentas->setObjectName("lblSinVentas");
    lblSinVentas->setAlignment(Qt::AlignCenter);
    lblSinVentas->hide();

    QVBoxLayout* layoutContenido = new QVBoxLayout(paginaHistorial);
    layoutContenido->setContentsMargins(40, 28, 40, 28);
    layoutContenido->setSpacing(16);
    layoutContenido->addLayout(layoutBarra);
    layoutContenido->addLayout(layoutMetricas);
    layoutContenido->addLayout(layoutFiltros);
    layoutContenido->addWidget(tabla, 1);
    layoutContenido->addWidget(lblSinVentas);
}

// ---------------- Pantalla de detalle ----------------

void VentanaHistorial::construirPaginaDetalle()
{
    paginaDetalle = new QWidget(this);

    btnVolverHistorial = new QPushButton("← Volver al historial", paginaDetalle);
    btnVolverHistorial->setObjectName("btnVolver");
    btnVolverHistorial->setCursor(Qt::PointingHandCursor);

    QLabel* lblTitulo = new QLabel("Detalle de la Venta", paginaDetalle);
    lblTitulo->setObjectName("lblTituloPantalla");

    lblDetalleTitulo = new QLabel("Venta N.º —", paginaDetalle);
    lblDetalleTitulo->setObjectName("lblDetalleTitulo");

    lblDetalleInfo = new QLabel(paginaDetalle);
    lblDetalleInfo->setObjectName("lblDetalleInfo");

    lblDetalleCajero = new QLabel(paginaDetalle);
    lblDetalleCajero->setObjectName("lblDetalleInfo");

    lblDetalleEstado = new QLabel("Venta guardada · no se puede editar ni borrar", paginaDetalle);
    lblDetalleEstado->setObjectName("lblEstadoVenta");

    QVBoxLayout* layoutTitulos = new QVBoxLayout();
    layoutTitulos->setSpacing(2);
    layoutTitulos->addWidget(lblTitulo);
    layoutTitulos->addWidget(lblDetalleTitulo);
    layoutTitulos->addWidget(lblDetalleInfo);
    layoutTitulos->addWidget(lblDetalleCajero);

    QHBoxLayout* layoutBarra = new QHBoxLayout();
    layoutBarra->setSpacing(16);
    layoutBarra->addWidget(btnVolverHistorial, 0, Qt::AlignTop);
    layoutBarra->addLayout(layoutTitulos, 1);
    layoutBarra->addWidget(lblDetalleEstado, 0, Qt::AlignTop);

    QLabel* lblFormato = new QLabel("Formato:", paginaDetalle);
    cmbFormato = new QComboBox(paginaDetalle);
    cmbFormato->addItems({ "TXT", "HTML" });

    QHBoxLayout* layoutFormato = new QHBoxLayout();
    layoutFormato->setSpacing(8);
    layoutFormato->addWidget(lblFormato);
    layoutFormato->addWidget(cmbFormato);
    layoutFormato->addStretch();

    tablaDetalle = new QTableWidget(paginaDetalle);
    tablaDetalle->setColumnCount(4);
    tablaDetalle->setHorizontalHeaderLabels({ "Producto", "Cant.", "Precio usado", "Subtotal" });
    tablaDetalle->verticalHeader()->setVisible(false);
    tablaDetalle->verticalHeader()->setDefaultSectionSize(40);
    tablaDetalle->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaDetalle->setSelectionMode(QAbstractItemView::NoSelection);
    tablaDetalle->setFocusPolicy(Qt::NoFocus);
    tablaDetalle->setShowGrid(false);

    QHeaderView* cabeceraDetalle = tablaDetalle->horizontalHeader();
    cabeceraDetalle->setSectionResizeMode(QHeaderView::ResizeToContents);
    cabeceraDetalle->setSectionResizeMode(0, QHeaderView::Stretch);
    cabeceraDetalle->setHighlightSections(false);

    lblDetalleTotal = new QLabel(paginaDetalle);
    lblDetalleTotal->setObjectName("lblTotalDetalle");
    lblDetallePago = new QLabel(paginaDetalle);
    lblDetallePago->setObjectName("lblTotalDetalle");
    lblDetalleCambio = new QLabel(paginaDetalle);
    lblDetalleCambio->setObjectName("lblTotalDetalle");

    QGridLayout* layoutTotales = new QGridLayout();
    layoutTotales->setHorizontalSpacing(30);
    layoutTotales->setVerticalSpacing(4);
    layoutTotales->addWidget(new QLabel("Total", paginaDetalle), 0, 0);
    layoutTotales->addWidget(lblDetalleTotal, 0, 1);
    layoutTotales->addWidget(new QLabel("Pagó en efectivo", paginaDetalle), 1, 0);
    layoutTotales->addWidget(lblDetallePago, 1, 1);
    layoutTotales->addWidget(new QLabel("Cambio", paginaDetalle), 2, 0);
    layoutTotales->addWidget(lblDetalleCambio, 2, 1);
    layoutTotales->setColumnStretch(2, 1);

    QLabel* lblNota = new QLabel("Los precios se guardaron al momento de la venta.", paginaDetalle);
    lblNota->setObjectName("lblNota");

    QVBoxLayout* layoutIzquierda = new QVBoxLayout();
    layoutIzquierda->setSpacing(12);
    layoutIzquierda->addLayout(layoutFormato);
    layoutIzquierda->addWidget(tablaDetalle, 1);
    layoutIzquierda->addLayout(layoutTotales);
    layoutIzquierda->addWidget(lblNota);

    QLabel* lblPreview = new QLabel("Vista previa del ticket", paginaDetalle);
    lblPreview->setObjectName("lblTag");

    vistaPrevia = new QTextBrowser(paginaDetalle);
    vistaPrevia->setObjectName("vistaPrevia");
    vistaPrevia->setFont(QFont("Courier New", 10));

    QVBoxLayout* layoutDerecha = new QVBoxLayout();
    layoutDerecha->setSpacing(8);
    layoutDerecha->addWidget(lblPreview);
    layoutDerecha->addWidget(vistaPrevia, 1);

    QHBoxLayout* layoutContenido = new QHBoxLayout();
    layoutContenido->setSpacing(24);
    layoutContenido->addLayout(layoutIzquierda, 3);
    layoutContenido->addLayout(layoutDerecha, 2);

    bannerMensaje = new QFrame(paginaDetalle);
    bannerMensaje->setObjectName("bannerMensaje");
    lblMensaje = new QLabel(bannerMensaje);
    lblMensaje->setObjectName("lblMensaje");
    lblMensaje->setWordWrap(true);

    btnReintentarImpresion = new QPushButton("Reintentar impresión", bannerMensaje);
    btnReintentarImpresion->setObjectName("btnReintentarImpresion");
    btnReintentarImpresion->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutBanner = new QHBoxLayout(bannerMensaje);
    layoutBanner->setContentsMargins(14, 10, 12, 10);
    layoutBanner->addWidget(lblMensaje, 1);
    layoutBanner->addWidget(btnReintentarImpresion);
    bannerMensaje->hide();

    btnImprimir = new QPushButton("Imprimir ticket / factura", paginaDetalle);
    btnImprimir->setObjectName("btnImprimir");
    btnImprimir->setCursor(Qt::PointingHandCursor);

    btnGuardar = new QPushButton("Guardar copia...", paginaDetalle);
    btnGuardar->setObjectName("btnGuardar");
    btnGuardar->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutBotones = new QHBoxLayout();
    layoutBotones->addStretch();
    layoutBotones->addWidget(btnGuardar);
    layoutBotones->addWidget(btnImprimir);

    QVBoxLayout* layoutPrincipal = new QVBoxLayout(paginaDetalle);
    layoutPrincipal->setContentsMargins(40, 28, 40, 28);
    layoutPrincipal->setSpacing(14);
    layoutPrincipal->addLayout(layoutBarra);
    layoutPrincipal->addWidget(bannerMensaje);
    layoutPrincipal->addLayout(layoutContenido, 1);
    layoutPrincipal->addLayout(layoutBotones);
}

// ---------------- Pantalla de error de carga ----------------

void VentanaHistorial::construirPaginaError()
{
    paginaError = new QWidget(this);

    QLabel* lblTitulo = new QLabel("Historial de Ventas", paginaError);
    lblTitulo->setObjectName("lblTituloPantalla");

    QLabel* lblErrorTitulo = new QLabel("No se pudo cargar el historial", paginaError);
    lblErrorTitulo->setObjectName("lblErrorTitulo");

    lblErrorCarga = new QLabel(paginaError);
    lblErrorCarga->setObjectName("lblErrorCarga");
    lblErrorCarga->setWordWrap(true);

    btnReintentarCarga = new QPushButton("Reintentar", paginaError);
    btnReintentarCarga->setObjectName("btnFiltrar");
    btnReintentarCarga->setCursor(Qt::PointingHandCursor);

    btnVolverError = new QPushButton("Volver al Menú Principal", paginaError);
    btnVolverError->setObjectName("btnVolver");
    btnVolverError->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutBotones = new QHBoxLayout();
    layoutBotones->setSpacing(10);
    layoutBotones->addWidget(btnReintentarCarga);
    layoutBotones->addWidget(btnVolverError);
    layoutBotones->addStretch();

    QVBoxLayout* layoutContenido = new QVBoxLayout(paginaError);
    layoutContenido->setContentsMargins(60, 50, 60, 50);
    layoutContenido->setSpacing(14);
    layoutContenido->addWidget(lblTitulo);
    layoutContenido->addSpacing(20);
    layoutContenido->addWidget(lblErrorTitulo);
    layoutContenido->addWidget(lblErrorCarga);
    layoutContenido->addSpacing(10);
    layoutContenido->addLayout(layoutBotones);
    layoutContenido->addStretch();
}

// ---------------- Estilos ----------------

void VentanaHistorial::aplicarEstilos()
{
    setStyleSheet(R"(
        VentanaHistorial { background-color: #fff8ee; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTituloPantalla { font-size: 26px; font-weight: bold; }
        QLabel#lblSubtituloPantalla { color: #7a6650; }
        QLabel#lblTag { background-color: #fff3e0; color: #8a6a4f; border: 1px solid #f0d9b5; border-radius: 10px; padding: 5px 12px; font-size: 11px; font-weight: bold; }
        QLabel#lblEtiquetaMetrica { font-size: 11px; font-weight: bold; color: #8a6a4f; }
        QLabel#lblDetalleMetrica { font-size: 11px; color: #8a7560; }
        QFrame#tarjetaMetrica { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QPushButton#btnVolver { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }
        QPushButton#btnVolver:hover { background-color: #fffaf3; border: 1px solid #e67e22; }
        QPushButton#btnFiltrar { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 9px 18px; font-weight: bold; }
        QPushButton#btnFiltrar:hover { background-color: #cf6d17; }
        QPushButton#btnLimpiar { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }
        QPushButton#btnLimpiar:hover { border: 1px solid #e67e22; }
        QPushButton#btnImprimir { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 11px 22px; font-weight: bold; }
        QPushButton#btnImprimir:hover { background-color: #cf6d17; }
        QPushButton#btnGuardar { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 11px 22px; font-weight: bold; }
        QPushButton#btnGuardar:hover { border: 1px solid #e67e22; }
        QPushButton#btnVerTicket { background-color: #ffffff; color: #d35400; border: 1px solid #e67e22; border-radius: 6px; padding: 6px 12px; font-weight: bold; }
        QPushButton#btnVerTicket:hover { background-color: #e67e22; color: #ffffff; }
        QPushButton#btnReintentarImpresion { background-color: #c0392b; color: #ffffff; border: none; border-radius: 6px; padding: 7px 14px; font-weight: bold; }
        QPushButton#btnReintentarImpresion:hover { background-color: #a93226; }
        QLineEdit, QDateEdit, QComboBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 7px; padding: 7px 10px; }
        QLineEdit:focus, QDateEdit:focus, QComboBox:focus { border: 1px solid #e67e22; }
        QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QTableWidget::item { border-bottom: 1px solid #f3e6d0; padding: 6px; }
        QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; border-bottom: 1px solid #f0d9b5; padding: 10px; }
        QTextBrowser#vistaPrevia { background-color: #fffdf9; border: 1px solid #f0d9b5; border-radius: 12px; padding: 10px; }
        QLabel#lblDetalleTitulo { font-size: 20px; font-weight: bold; }
        QLabel#lblDetalleInfo { color: #7a6650; }
        QLabel#lblEstadoVenta { background-color: #fff3e0; color: #8a6a4f; border: 1px solid #f0d9b5; border-radius: 10px; padding: 5px 12px; font-size: 11px; font-weight: bold; }
        QLabel#lblTotalDetalle { font-size: 16px; font-weight: bold; }
        QLabel#lblNota { color: #8a7560; font-size: 12px; }
        QLabel#lblSinVentas { color: #8a7560; font-size: 14px; padding: 20px; }
        QFrame#bannerMensaje { background-color: #e8f5e9; border: 1px solid #c8e6c9; border-radius: 8px; }
        QFrame#bannerMensaje[tipo="error"] { background-color: #fdecea; border: 1px solid #e6a59c; }
        QLabel#lblMensaje { color: #1e8449; }
        QLabel#lblMensaje[tipo="error"] { color: #c0392b; }
        QLabel#lblErrorTitulo { font-size: 20px; font-weight: bold; color: #c0392b; }
        QLabel#lblErrorCarga { color: #7a6650; font-size: 15px; }
    )");
}

// ---------------- Carga y filtros ----------------

void VentanaHistorial::cargarHistorial()
{
    if (!historial_)
    {
        lblErrorCarga->setText("El historial no está disponible porque no se pudo inicializar el almacén.");
        paginas->setCurrentWidget(paginaError);
        return;
    }

    try
    {
        const std::vector<VentasCobro> ventas = historial_->listar();

        if (ventas.empty())
        {
            const QDate hoy = QDate::currentDate();
            fechaDesde->setDate(hoy);
            fechaHasta->setDate(hoy);
        }
        else
        {
            QDate minima = ventas.front().fecha().date();
            QDate maxima = minima;

            for (const VentasCobro& venta : ventas)
            {
                const QDate fecha = venta.fecha().date();
                if (fecha < minima) minima = fecha;
                if (fecha > maxima) maxima = fecha;
            }

            fechaDesde->setDate(minima);
            fechaHasta->setDate(maxima);
        }

        actualizarMetricas();
        paginas->setCurrentWidget(paginaHistorial);
        aplicarFiltro();
    }
    catch (const std::exception& e)
    {
        lblErrorCarga->setText(
            "Error al leer el archivo de ventas: " + QString::fromUtf8(e.what()) +
            "\n\nNo se modificó ningún dato.");
        paginas->setCurrentWidget(paginaError);
    }
}

void VentanaHistorial::actualizarMetricas()
{
    if (!historial_)
        return;

    lblVentasGuardadas->setText(QString::number(historial_->cantidadVentas()));
    lblTotalVendido->setText(Formato::colones(historial_->totalVendido()));

    VentasCobro ultima;
    if (historial_->ultimaVenta(ultima))
        lblUltimaVenta->setText(ultima.fecha().toString("dd/MM HH:mm"));
    else
        lblUltimaVenta->setText("—");
}

void VentanaHistorial::limpiarFiltros()
{
    txtBuscar->clear();
    cargarHistorial();
}

void VentanaHistorial::aplicarFiltro()
{
    if (!historial_)
        return;

    const QDate desde = fechaDesde->date();
    const QDate hasta = fechaHasta->date();

    if (desde > hasta)
    {
        QMessageBox::warning(this, "Fechas inválidas",
            "La fecha inicial no puede ser mayor que la fecha final.");
        return;
    }

    try
    {
        llenarTabla(historial_->filtrar(txtBuscar->text(), desde, hasta));
    }
    catch (const std::exception& e)
    {
        lblErrorCarga->setText(
            "Error al leer el archivo de ventas: " + QString::fromUtf8(e.what()) +
            "\n\nNo se modificó ningún dato.");
        paginas->setCurrentWidget(paginaError);
    }
}

void VentanaHistorial::llenarTabla(const std::vector<VentasCobro>& ventas)
{
    tabla->setRowCount(0);
    tabla->setRowCount(static_cast<int>(ventas.size()));

    for (int fila = 0; fila < static_cast<int>(ventas.size()); fila++)
    {
        const VentasCobro& venta = ventas[fila];

        tabla->setItem(fila, 0, crearItem(Formato::numero(venta.numero()), true));
        tabla->setItem(fila, 1, crearItem(venta.fecha().toString("dd/MM/yyyy HH:mm"), false));
        tabla->setItem(fila, 2, crearItem(venta.cajero(), false));
        tabla->setItem(fila, 3, crearItem(QString::number(static_cast<int>(venta.lineas().size())) + " art.", true));
        tabla->setItem(fila, 4, crearItem(Formato::colones(venta.total()), true));
        tabla->setItem(fila, 5, crearItem(Formato::colones(venta.cambio()), true));

        QPushButton* btnVer = new QPushButton("Ver ticket / factura");
        btnVer->setObjectName("btnVerTicket");
        btnVer->setCursor(Qt::PointingHandCursor);
        btnVer->setMinimumSize(150, 32);

        const int numero = venta.numero();
        connect(btnVer, &QPushButton::clicked, this, [this, numero]()
            {
                mostrarDetalle(numero);
            });

        QWidget* contenedor = new QWidget();
        QHBoxLayout* layoutCelda = new QHBoxLayout(contenedor);
        layoutCelda->setContentsMargins(8, 4, 8, 4);
        layoutCelda->addWidget(btnVer, 0, Qt::AlignCenter);
        tabla->setCellWidget(fila, 6, contenedor);
    }

    lblSinVentas->setVisible(ventas.empty());
    tabla->setVisible(!ventas.empty());
}

// ---------------- Detalle e impresión ----------------

void VentanaHistorial::mostrarDetalle(int numero)
{
    if (!historial_)
        return;

    try
    {
        ventaActual_ = historial_->buscar(numero);
        hayVentaActual_ = true;

        lblDetalleTitulo->setText("Venta N.º " + Formato::numero(ventaActual_.numero()));
        lblDetalleInfo->setText(ventaActual_.fecha().toString("dd/MM/yyyy  HH:mm"));
        lblDetalleCajero->setText("Cajero: " + ventaActual_.cajero());

        tablaDetalle->setRowCount(0);
        tablaDetalle->setRowCount(static_cast<int>(ventaActual_.lineas().size()));

        for (int fila = 0; fila < static_cast<int>(ventaActual_.lineas().size()); fila++)
        {
            const Linea& linea = ventaActual_.lineas()[fila];
            tablaDetalle->setItem(fila, 0, crearItem(linea.nombre(), false));
            tablaDetalle->setItem(fila, 1, crearItem(QString::number(linea.cantidad()), true));
            tablaDetalle->setItem(fila, 2, crearItem(Formato::colones(linea.precio()), true));
            tablaDetalle->setItem(fila, 3, crearItem(Formato::colones(linea.subtotal()), true));
        }

        lblDetalleTotal->setText(Formato::colones(ventaActual_.total()));
        lblDetallePago->setText(Formato::colones(ventaActual_.pago()));
        lblDetalleCambio->setText(Formato::colones(ventaActual_.cambio()));

        ocultarMensaje();
        btnImprimir->setText("Imprimir ticket / factura");
        cmbFormato->setCurrentIndex(0);
        actualizarVistaPrevia();
        paginas->setCurrentWidget(paginaDetalle);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "No se pudo abrir la venta",
            QString::fromUtf8(e.what()));
    }
}

void VentanaHistorial::actualizarVistaPrevia()
{
    if (!historial_ || !hayVentaActual_)
        return;

    if (cmbFormato->currentText() == "HTML")
    {
        TicketHtml ticket("La Pulpe");
        vistaPrevia->setHtml(historial_->generar(ventaActual_, ticket));
    }
    else
    {
        TicketTxt ticket("La Pulpe");
        vistaPrevia->setPlainText(historial_->generar(ventaActual_, ticket));
        vistaPrevia->setFont(QFont("Courier New", 10));
    }
}

void VentanaHistorial::imprimirTicket()
{
    if (!historial_ || !hayVentaActual_)
        return;

    try
    {
        const bool html = (cmbFormato->currentText() == "HTML");
        const QString extension = html ? "html" : "txt";
        const QString carpeta = "tickets";

        if (!QDir().mkpath(carpeta))
            throw ErrorHistorial("No se pudo crear la carpeta de tickets.");

        const QString ruta = carpeta + "/ticket_" +
            Formato::numero(ventaActual_.numero()) + "." + extension;

        if (html)
        {
            TicketHtml ticket("La Pulpe");
            historial_->imprimir(ventaActual_, ticket, ruta);
        }
        else
        {
            TicketTxt ticket("La Pulpe");
            historial_->imprimir(ventaActual_, ticket, ruta);
        }

        mostrarMensaje("Ticket / factura N.º " + Formato::numero(ventaActual_.numero()) +
            " generado en formato " + extension.toUpper() + " — la venta no cambió.",
            "exito");
        btnImprimir->setText("Imprimir de nuevo");
    }
    catch (const std::exception& e)
    {
        mostrarMensaje("No se pudo generar el ticket / factura. "
            "La venta sigue guardada, intenta de nuevo. (" +
            QString::fromUtf8(e.what()) + ")", "error");
    }
}

void VentanaHistorial::guardarCopia()
{
    if (!historial_ || !hayVentaActual_)
        return;

    const bool html = (cmbFormato->currentText() == "HTML");
    const QString filtro = html ? "Ticket HTML (*.html)" : "Ticket TXT (*.txt)";
    const QString nombreSugerido = "ticket_" +
        Formato::numero(ventaActual_.numero()) + (html ? ".html" : ".txt");

    const QString ruta = QFileDialog::getSaveFileName(
        this, "Guardar copia del ticket", nombreSugerido, filtro);

    if (ruta.isEmpty())
        return;

    try
    {
        if (html)
        {
            TicketHtml ticket("La Pulpe");
            historial_->imprimir(ventaActual_, ticket, ruta);
        }
        else
        {
            TicketTxt ticket("La Pulpe");
            historial_->imprimir(ventaActual_, ticket, ruta);
        }

        mostrarMensaje("Copia guardada correctamente en: " + ruta, "exito", false);
    }
    catch (const std::exception& e)
    {
        mostrarMensaje("No se pudo guardar la copia. (" +
            QString::fromUtf8(e.what()) + ")", "error", false);
    }
}

void VentanaHistorial::mostrarMensaje(const QString& texto, const QString& tipo, bool permitirReintento)
{
    lblMensaje->setText(texto);

    bannerMensaje->setProperty("tipo", tipo);
    lblMensaje->setProperty("tipo", tipo);
    bannerMensaje->style()->unpolish(bannerMensaje);
    bannerMensaje->style()->polish(bannerMensaje);
    lblMensaje->style()->unpolish(lblMensaje);
    lblMensaje->style()->polish(lblMensaje);

    btnReintentarImpresion->setVisible(tipo == "error" && permitirReintento);
    bannerMensaje->show();
}

void VentanaHistorial::ocultarMensaje()
{
    bannerMensaje->hide();
    btnReintentarImpresion->hide();
}
