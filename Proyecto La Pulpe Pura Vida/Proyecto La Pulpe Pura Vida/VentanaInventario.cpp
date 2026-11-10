#include "VentanaInventario.h"
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QLocale>
#include <QComboBox>

// =====================================================
// CONSTRUCTOR
// =====================================================
VentanaInventario::VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent)
    : QWidget(parent), gestor(gestorCompartido), soloReposicion(false), textoBusqueda("")
{
    setWindowTitle("La Pulpe Pura Vida — Inventario");
    resize(1050, 750);

    configurarInterfaz();
    estilizarVentana();

    if (gestor != nullptr) {
        gestor->cargarDesdeArchivo("productos.json");
        // Cargamos exactamente los 9 productos oficiales del prototipo
        if (gestor->getProductosActivos().empty()) {
            gestor->agregarProducto(Producto("001", "Arroz 1 kg", "Abarrotes", 1450.0, 24));
            gestor->agregarProducto(Producto("002", "Frijoles negros 800 g", "Abarrotes", 1200.0, 18));
            gestor->agregarProducto(Producto("003", "Aceite de girasol 1 L", "Abarrotes", 2650.0, 3));   // Stock bajo
            gestor->agregarProducto(Producto("004", "Leche entera 1 L", "Lacteos", 1150.0, 30));
            gestor->agregarProducto(Producto("005", "Cafe molido 500 g", "Abarrotes", 3900.0, 9));
            gestor->agregarProducto(Producto("006", "Azucar 2 kg", "Abarrotes", 2300.0, 0));             // Agotado
            gestor->agregarProducto(Producto("007", "Huevos carton de 30", "Frescos", 5200.0, 14));
            gestor->agregarProducto(Producto("009", "Atun en lata", "Enlatados", 1350.0, 16));
            gestor->agregarProducto(Producto("010", "Pan de caja", "Panaderia", 1800.0, 12));
            gestor->guardarEnArchivo("productos.json");
        }
    }

    actualizarVista();
}

// =====================================================
// CONFIGURACIÓN DE LA INTERFAZ
// =====================================================
void VentanaInventario::configurarInterfaz() {
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(40, 30, 40, 20);
    layoutPrincipal->setSpacing(16);

    // 1. Cabecera
    QHBoxLayout* lHeader = new QHBoxLayout();
    QVBoxLayout* lTitulos = new QVBoxLayout();

    QLabel* lblTitulo = new QLabel("Inventario", this);
    lblTitulo->setStyleSheet("font-size: 28px; font-weight: 900; color: #872017;");

    QLabel* lblSub = new QLabel("Tu mercaderia, de un vistazo. Haz clic en cualquier tarjeta para editar o reponer stock.", this);
    lblSub->setStyleSheet("font-size: 13px; color: #7a6650;");

    lTitulos->addWidget(lblTitulo);
    lTitulos->addWidget(lblSub);

    QPushButton* btnVolver = new QPushButton("← Menu Principal", this);
    btnVolver->setObjectName("btnVolver");
    btnVolver->setCursor(Qt::PointingHandCursor);
    connect(btnVolver, &QPushButton::clicked, this, &QWidget::close);

    QPushButton* btnNuevo = new QPushButton("+ Nuevo producto", this);
    btnNuevo->setObjectName("btnNuevo");
    btnNuevo->setCursor(Qt::PointingHandCursor);
    connect(btnNuevo, &QPushButton::clicked, this, &VentanaInventario::abrirModalNuevoProducto);

    lHeader->addLayout(lTitulos);
    lHeader->addStretch();
    lHeader->addWidget(btnVolver);
    lHeader->addWidget(btnNuevo);
    layoutPrincipal->addLayout(lHeader);

    // 2. Tarjetas de métricas
    QHBoxLayout* layoutCards = new QHBoxLayout();
    layoutCards->setSpacing(18);

    auto crearCardMetrica = [](const QString& etiqueta, QLabel*& lblNum, const QString& detalle, const QString& colorNum) {
        QFrame* card = new QFrame();
        card->setObjectName("cardMetrica");
        QVBoxLayout* v = new QVBoxLayout(card);
        v->setContentsMargins(18, 14, 18, 14);
        v->setSpacing(2);

        QLabel* lblE = new QLabel(etiqueta, card);
        lblE->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a6a4f;");

        lblNum = new QLabel("0", card);
        lblNum->setStyleSheet("font-size: 28px; font-weight: 900; color: " + colorNum + ";");

        QLabel* lblD = new QLabel(detalle, card);
        lblD->setStyleSheet("font-size: 11px; color: #8a7560;");

        v->addWidget(lblE);
        v->addWidget(lblNum);
        v->addWidget(lblD);
        return card;
        };

    QWidget* cardUnidades = crearCardMetrica("UNIDADES DISPONIBLES", lblTotalUnidades, "", "#16a34a");
    lblDetalleUnidades = cardUnidades->findChildren<QLabel*>().at(2);
    layoutCards->addWidget(cardUnidades);
    layoutCards->addWidget(crearCardMetrica("PRODUCTOS AGOTADOS", lblTotalAgotados, "necesita reposicion", "#dc2626"));
    layoutCards->addWidget(crearCardMetrica("VALOR DE MERCADERIA", lblValorTotal, "Precio de venta x existencias activas", "#1f2937"));
    layoutPrincipal->addLayout(layoutCards);

    // 3. Banner de reposición
    QFrame* banner = new QFrame(this);
    banner->setObjectName("bannerAlerta");
    QHBoxLayout* lBanner = new QHBoxLayout(banner);
    lBanner->setContentsMargins(16, 10, 16, 10);

    lblTextoAlerta = new QLabel(banner);
    lblTextoAlerta->setStyleSheet("font-size: 12px; font-weight: bold; color: #92400e;");

    btnVerReposicionBanner = new QPushButton("Ver reposicion →", banner);
    btnVerReposicionBanner->setObjectName("btnLinkReposicion");
    btnVerReposicionBanner->setCursor(Qt::PointingHandCursor);
    connect(btnVerReposicionBanner, &QPushButton::clicked, this, &VentanaInventario::setFiltroReposicion);

    lBanner->addWidget(lblTextoAlerta);
    lBanner->addStretch();
    lBanner->addWidget(btnVerReposicionBanner);
    layoutPrincipal->addWidget(banner);

    // 4. Barra de búsqueda y filtros
    QHBoxLayout* lFiltros = new QHBoxLayout();
    txtBuscador = new QLineEdit(this);
    txtBuscador->setPlaceholderText("🔍 Buscar por codigo o nombre...");
    txtBuscador->setObjectName("txtBuscador");
    connect(txtBuscador, &QLineEdit::textChanged, this, &VentanaInventario::filtrarPorTexto);

    btnFiltroTodos = new QPushButton("Todos", this);
    btnFiltroTodos->setObjectName("btnPestanaActiva");
    btnFiltroTodos->setCursor(Qt::PointingHandCursor);
    connect(btnFiltroTodos, &QPushButton::clicked, this, &VentanaInventario::setFiltroTodos);

    btnFiltroReposicion = new QPushButton("Por reponer", this);
    btnFiltroReposicion->setObjectName("btnPestanaInactiva");
    btnFiltroReposicion->setCursor(Qt::PointingHandCursor);
    connect(btnFiltroReposicion, &QPushButton::clicked, this, &VentanaInventario::setFiltroReposicion);

    lFiltros->addWidget(txtBuscador, 1);
    lFiltros->addSpacing(15);
    lFiltros->addWidget(btnFiltroTodos);
    lFiltros->addWidget(btnFiltroReposicion);
    layoutPrincipal->addLayout(lFiltros);

    // 5. Scroll del Estante
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (scroll->verticalScrollBar()) {
        scroll->verticalScrollBar()->setStyleSheet(
            "QScrollBar:vertical { width: 6px; background: transparent; }"
            "QScrollBar::handle:vertical { background: #d4b89b; border-radius: 3px; }"
        );
    }

    contenedorCards = new QWidget();
    contenedorCards->setStyleSheet("background: transparent;");
    gridProductos = new QGridLayout(contenedorCards);
    gridProductos->setSpacing(16);
    gridProductos->setContentsMargins(0, 8, 0, 8);

    scroll->setWidget(contenedorCards);
    layoutPrincipal->addWidget(scroll, 1);

    // 6. Pie de página
    QHBoxLayout* lPie = new QHBoxLayout();
    lblResumenPie = new QLabel("9 productos - 126 unidades activas", this);
    lblResumenPie->setStyleSheet("font-size: 11px; color: #8a7560;");

    QLabel* lblReglaNegocio = new QLabel("Regla del Negocio: Agotado no es desactivado.", this);
    lblReglaNegocio->setStyleSheet("font-size: 11px; font-style: italic; color: #8a7560;");

    lPie->addWidget(lblResumenPie);
    lPie->addStretch();
    lPie->addWidget(lblReglaNegocio);
    layoutPrincipal->addLayout(lPie);
}

// =====================================================
// ESTILOS DE LA VENTANA
// =====================================================
void VentanaInventario::estilizarVentana() {
    setStyleSheet(R"(
        VentanaInventario {
            background-color: #fdf8f2;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QPushButton#btnVolver {
            background-color: #ffffff;
            color: #4a3424;
            border: 1px solid #d4c4b2;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
            padding: 9px 16px;
        }
        QPushButton#btnVolver:hover {
            background-color: #f5ebe0;
        }
        QPushButton#btnNuevo {
            background-color: #872017;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
            padding: 9px 18px;
        }
        QPushButton#btnNuevo:hover {
            background-color: #9e281e;
        }
        QFrame#cardMetrica {
            background-color: #ffffff;
            border: 1px solid #eadbc8;
            border-radius: 12px;
        }
        QFrame#bannerAlerta {
            background-color: #fef3c7;
            border: 1px solid #fde68a;
            border-radius: 8px;
        }
        QPushButton#btnLinkReposicion {
            background: transparent;
            color: #92400e;
            border: none;
            font-size: 12px;
            font-weight: bold;
            text-decoration: underline;
        }
        QLineEdit#txtBuscador {
            background-color: #ffffff;
            border: 1px solid #d4c4b2;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 13px;
            color: #3b2a1a;
        }
        QPushButton#btnPestanaActiva {
            background-color: #872017;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 7px 16px;
            border: none;
        }
        QPushButton#btnPestanaInactiva {
            background-color: #ffffff;
            color: #6b4f3b;
            font-weight: bold;
            border-radius: 6px;
            padding: 7px 16px;
            border: 1px solid #d4c4b2;
        }
    )");
}

// =====================================================
// CREAR CADA TARJETA INDIVIDUAL
// =====================================================
QWidget* VentanaInventario::crearCardProducto(const Producto& prod) {
    QFrame* card = new QFrame();
    card->setFixedSize(290, 185);
    card->setCursor(Qt::PointingHandCursor);

    bool esAgotado = (prod.getStock() == 0);
    bool esBajo = (prod.getStock() > 0 && prod.getStock() <= 5);

    QString estiloBorde = esAgotado
        ? "border: 2px dashed #f87171; background-color: #fff1f2;"
        : "border: 1px solid #eadbc8; background-color: #ffffff;";

    card->setStyleSheet(QString(
        "QFrame {"
        "   %1"
        "   border-radius: 12px;"
        "}"
        "QFrame:hover {"
        "   border: 2px solid #872017;"
        "}"
        "QLabel { background: transparent; border: none; font-family: 'Segoe UI'; }"
    ).arg(estiloBorde));

    QVBoxLayout* v = new QVBoxLayout(card);
    v->setContentsMargins(14, 12, 14, 12);
    v->setSpacing(3);

    // Fila superior: Código y botón "Reponer/Editar"
    QHBoxLayout* filaTop = new QHBoxLayout();
    QLabel* lblCod = new QLabel(QString::fromStdString(prod.getCodigo()), card);
    lblCod->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a7560;");

    QLabel* lblEditar = new QLabel("✏️ Reponer/Editar", card);
    lblEditar->setStyleSheet("font-size: 11px; font-weight: bold; color: #b45309;");

    filaTop->addWidget(lblCod);
    filaTop->addStretch();
    filaTop->addWidget(lblEditar);
    v->addLayout(filaTop);

    // Ilustración simbólica del producto en el centro
    QLabel* lblGrafico = new QLabel(card);
    lblGrafico->setAlignment(Qt::AlignCenter);
    lblGrafico->setFixedHeight(46);
    if (esAgotado) {
        lblGrafico->setText("AGOTADO");
        lblGrafico->setStyleSheet("background-color: #fee2e2; color: #991b1b; font-weight: bold; font-size: 11px; border: 1px solid #f87171; border-radius: 6px; padding: 4px 10px;");
    }
    else {
        QString textoCaja = QString::fromStdString(prod.getNombre()).toUpper().left(10);
        lblGrafico->setText(textoCaja);
        lblGrafico->setStyleSheet("background-color: #fdf8f2; color: #2b1810; font-weight: bold; font-size: 10px; border: 1px solid #2b1810; border-radius: 6px; padding: 4px 12px;");
    }
    v->addWidget(lblGrafico, 0, Qt::AlignCenter);

    // Nombre y categoría
    QLabel* lblNom = new QLabel(QString::fromStdString(prod.getNombre()), card);
    lblNom->setStyleSheet("font-size: 14px; font-weight: bold; color: #1f2937; margin-top: 4px;");
    lblNom->setWordWrap(true);

    QLabel* lblCat = new QLabel(QString::fromStdString(prod.getCategoria()), card);
    lblCat->setStyleSheet("font-size: 11px; color: #6b7280;");

    v->addWidget(lblNom);
    v->addWidget(lblCat);
    v->addStretch();

    // Fila inferior: Precio y Estado
    QHBoxLayout* filaBot = new QHBoxLayout();
    QLocale loc(QLocale::Spanish, QLocale::CostaRica);
    QLabel* lblPrecio = new QLabel("₡ " + loc.toString((int)prod.getPrecio()), card);
    lblPrecio->setStyleSheet("font-size: 15px; font-weight: 900; color: #111827;");

    QString txtEstado = esAgotado ? "0 unid.  ● Agotado" : (esBajo ? QString::number(prod.getStock()) + " unid.  ● Stock bajo" : QString::number(prod.getStock()) + " unid.  ● Disponible");
    QString colEstado = esAgotado ? "#dc2626" : (esBajo ? "#d97706" : "#16a34a");

    QLabel* lblEstado = new QLabel(txtEstado, card);
    lblEstado->setStyleSheet("font-size: 11px; font-weight: bold; color: " + colEstado + ";");

    filaBot->addWidget(lblPrecio);
    filaBot->addStretch();
    filaBot->addWidget(lblEstado);
    v->addLayout(filaBot);

    // Al hacer clic en cualquier parte de la tarjeta, abre el modal de edición/reposición
    std::string codActual = prod.getCodigo();
    QPushButton* btnTransparente = new QPushButton(card);
    btnTransparente->setGeometry(0, 0, 290, 185);
    btnTransparente->setStyleSheet("background: transparent; border: none;");
    btnTransparente->setCursor(Qt::PointingHandCursor);
    connect(btnTransparente, &QPushButton::clicked, this, [this, codActual]() {
        abrirModalDetalleProducto(codActual);
        });

    return card;
}

// =====================================================
// ACTUALIZAR ESTANTE Y MÉTRICAS
// =====================================================
void VentanaInventario::actualizarVista() {
    if (gestor == nullptr) return;

    QLayoutItem* item;
    while ((item = gridProductos->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto productos = gestor->getProductosActivos();
    int totalUnidades = 0;
    int agotados = 0;
    double valorTotal = 0.0;
    QStringList nombresAgotados;
    QStringList nombresBajos;

    int columna = 0;
    int fila = 0;
    int renderizados = 0;

    for (const auto& prod : productos) {
        totalUnidades += prod.getStock();
        valorTotal += (prod.getPrecio() * prod.getStock());

        if (prod.getStock() == 0) {
            agotados++;
            nombresAgotados.append(QString::fromStdString(prod.getNombre()));
        }
        else if (prod.getStock() <= 5) {
            nombresBajos.append(QString::fromStdString(prod.getNombre()) + " quedan " + QString::number(prod.getStock()));
        }

        // Filtro por pestaña "Por reponer"
        if (soloReposicion && prod.getStock() > 5) {
            continue;
        }

        // Filtro por texto en el buscador
        if (!textoBusqueda.isEmpty()) {
            QString cod = QString::fromStdString(prod.getCodigo()).toLower();
            QString nom = QString::fromStdString(prod.getNombre()).toLower();
            if (!cod.contains(textoBusqueda.toLower()) && !nom.contains(textoBusqueda.toLower())) {
                continue;
            }
        }

        QWidget* card = crearCardProducto(prod);
        gridProductos->addWidget(card, fila, columna);
        renderizados++;

        columna++;
        if (columna >= 3) {
            columna = 0;
            fila++;
        }
    }

    // Actualizar métricas superiores
    QLocale loc(QLocale::Spanish, QLocale::CostaRica);
    lblDetalleUnidades->setText(QString("en %1 productos").arg((int)productos.size()));
    lblTotalAgotados->setText(QString("%1").arg(agotados, 2, 10, QChar('0')));
    lblValorTotal->setText("₡" + loc.toString((int)valorTotal));

    // Texto de alerta de reposición
    QString aviso = "⚠️ Por reponer... ";
    if (!nombresAgotados.isEmpty()) aviso += nombresAgotados.join(" / ") + " agotadas";
    if (!nombresAgotados.isEmpty() && !nombresBajos.isEmpty()) aviso += " / ";
    if (!nombresBajos.isEmpty()) aviso += nombresBajos.join(" / ");
    if (nombresBajos.isEmpty() && nombresAgotados.isEmpty()) aviso = "✅ Todas las existencias están en niveles óptimos.";
    lblTextoAlerta->setText(aviso);

    // Pie de página
    lblResumenPie->setText(QString::number(renderizados) + " productos · " + QString::number(totalUnidades) + " unidades");
}

// =====================================================
// MODAL: REPONER O EDITAR PRODUCTO (AL TOCAR LA TARJETA)
// =====================================================
void VentanaInventario::abrirModalDetalleProducto(const std::string& codigo) {
    Producto* prod = gestor->buscarPorCodigo(codigo);
    if (prod == nullptr) return;

    QDialog modal(this);
    modal.setWindowTitle("Reponer / Editar Producto — La Pulpe");
    modal.setFixedWidth(420);
    modal.setStyleSheet("background-color: #fdf8f2; font-family: 'Segoe UI'; font-size: 13px; color: #3b2a1a;");

    QVBoxLayout* layout = new QVBoxLayout(&modal);
    layout->setContentsMargins(25, 20, 25, 20);
    layout->setSpacing(12);

    QLabel* titulo = new QLabel("Editar: " + QString::fromStdString(prod->getNombre()), &modal);
    titulo->setStyleSheet("font-size: 18px; font-weight: 900; color: #872017;");
    layout->addWidget(titulo);

    QLabel* subtitulo = new QLabel("Código: " + QString::fromStdString(prod->getCodigo()) + " (Inmutable) | Categoría: " + QString::fromStdString(prod->getCategoria()), &modal);
    subtitulo->setStyleSheet("color: #7a6650; font-size: 12px; margin-bottom: 8px;");
    layout->addWidget(subtitulo);

    QFormLayout form;

    QDoubleSpinBox spinPrecio(&modal);
    spinPrecio.setRange(0.0, 1000000.0);  // ← permite 0 para que la validación lo atrape
    spinPrecio.setValue(prod->getPrecio());
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinSumarStock(&modal);
    spinSumarStock.setRange(0, 500);
    spinSumarStock.setValue(0);
    spinSumarStock.setSuffix(" unid.");

    QLabel* lblStockActual = new QLabel(QString::number(prod->getStock()) + " unidades actualmente en bodega", &modal);
    lblStockActual->setStyleSheet("font-weight: bold; color: #166534;");

    QString estiloInput = "background: white; border: 1px solid #d4c4b2; border-radius: 6px; padding: 7px;";
    spinPrecio.setStyleSheet(estiloInput);
    spinSumarStock.setStyleSheet(estiloInput);

    form.addRow("Stock actual:", lblStockActual);
    form.addRow("Reponer (+ sumar stock):", &spinSumarStock);
    form.addRow("Precio de venta *:", &spinPrecio);
    layout->addLayout(&form);

    QHBoxLayout* lBotones = new QHBoxLayout();

    // Botón de Borrado Lógico
    QPushButton btnBorradoLogico(prod->getActivo() ? "Desactivar Producto" : "Reactivar Producto", &modal);
    btnBorradoLogico.setStyleSheet("background-color: #d97706; color: white; font-weight: bold; border-radius: 6px; padding: 10px;");
    btnBorradoLogico.setCursor(Qt::PointingHandCursor);

    QPushButton btnGuardar("Guardar cambios", &modal);
    btnGuardar.setStyleSheet("background-color: #872017; color: white; font-weight: bold; border-radius: 6px; padding: 10px;");
    btnGuardar.setCursor(Qt::PointingHandCursor);

    lBotones->addWidget(&btnBorradoLogico);
    lBotones->addWidget(&btnGuardar);
    layout->addLayout(lBotones);

    // Evento Guardar Reposición y Precio
    connect(&btnGuardar, &QPushButton::clicked, [&]() {
        double nuevoPrecio = spinPrecio.value();
        int sumar = spinSumarStock.value();

        if (nuevoPrecio <= 0.0) {
            QMessageBox::warning(&modal, "Precio inválido", "El precio debe ser estrictamente mayor a ₡0.");
            return;
        }

        prod->setPrecio(nuevoPrecio);
        if (sumar > 0) {
            prod->agregarStock(sumar);
        }

        gestor->guardarEnArchivo("productos.json");
        actualizarVista();
        QMessageBox::information(this, "Actualizado", "El producto se actualizó y guardó en disco.");
        modal.accept();
        });

    // Evento Borrado Lógico
    connect(&btnBorradoLogico, &QPushButton::clicked, [&]() {
        bool nuevoEstado = !prod->getActivo();
        gestor->cambiarEstado(codigo, nuevoEstado);
        gestor->guardarEnArchivo("productos.json");
        modal.accept();
        actualizarVista();
        });

    modal.exec();
}

// =====================================================
// MODAL NUEVO PRODUCTO
// =====================================================
void VentanaInventario::abrirModalNuevoProducto() {
    QDialog modal(this);
    modal.setWindowTitle("Nuevo Producto — La Pulpe");
    modal.setFixedWidth(420);
    modal.setStyleSheet("background-color: #fdf8f2; font-family: 'Segoe UI'; font-size: 13px; color: #3b2a1a;");

    QVBoxLayout* layout = new QVBoxLayout(&modal);
    layout->setContentsMargins(25, 20, 25, 20);
    layout->setSpacing(12);

    QLabel* titulo = new QLabel("Nuevo producto", &modal);
    titulo->setStyleSheet("font-size: 20px; font-weight: 900; color: #872017;");
    layout->addWidget(titulo);

    QLabel* subtitulo = new QLabel("Ingresa los datos del artículo para sumarlo a la bodega.", &modal);
    subtitulo->setStyleSheet("color: #7a6650; font-size: 12px; margin-bottom: 6px;");
    layout->addWidget(subtitulo);

    QFormLayout form;
    QLineEdit txtCodigo(&modal);
    QLineEdit txtNombre(&modal);
    QComboBox cmbCategoria(&modal);
    cmbCategoria.addItems({ "Lácteos", "Abarrotes", "Snacks y galletas", "Bebidas" });

    QDoubleSpinBox spinPrecio(&modal);
    spinPrecio.setRange(0.0, 1000000.0);  // ← permite 0 para que la validación lo atrape
    spinPrecio.setValue(1000.0);
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinStock(&modal);
    spinStock.setRange(-1000, 10000);
    spinStock.setValue(10);
    spinStock.setSuffix(" unid.");

    QString estiloInput = "background: white; border: 1px solid #d4c4b2; border-radius: 6px; padding: 7px;";
    txtCodigo.setStyleSheet(estiloInput);
    txtNombre.setStyleSheet(estiloInput);
    cmbCategoria.setStyleSheet(estiloInput);
    spinPrecio.setStyleSheet(estiloInput);
    spinStock.setStyleSheet(estiloInput);

    form.addRow("CÓDIGO ÚNICO *:", &txtCodigo);
    form.addRow("NOMBRE DEL PRODUCTO *:", &txtNombre);
    form.addRow("CATEGORÍA *:", &cmbCategoria);
    form.addRow("PRECIO DE VENTA *:", &spinPrecio);
    form.addRow("STOCK INICIAL *:", &spinStock);
    layout->addLayout(&form);

    QPushButton btnAceptar("Guardar producto", &modal);
    btnAceptar.setStyleSheet("background-color: #872017; color: white; font-weight: bold; border-radius: 8px; padding: 11px; margin-top: 10px;");
    btnAceptar.setCursor(Qt::PointingHandCursor);
    layout->addWidget(&btnAceptar);

    connect(&btnAceptar, &QPushButton::clicked, [&]() {
        std::string cod = txtCodigo.text().trimmed().toStdString();
        std::string nom = txtNombre.text().trimmed().toStdString();
        std::string cat = cmbCategoria.currentText().toStdString();
        double precio = spinPrecio.value();
        int stock = spinStock.value();

        // Validaciones rigurosas
        if (cod.empty() || nom.empty()) {
            QMessageBox::warning(&modal, "Campos obligatorios", "El código y el nombre no pueden quedar vacíos.");
            return;
        }

        if (!gestor->validarCodigoUnico(cod)) {
            QMessageBox::warning(&modal, "Código duplicado", "Ya existe un producto registrado con ese código único.");
            return;
        }

        

        Producto nuevo(cod, nom, cat, precio, stock);

        if (!nuevo.validarPrecio(precio)) {
            QMessageBox::warning(&modal, "Precio inválido", "El precio debe ser mayor a 0.");
            return;
        }
        if (!nuevo.validarStock(stock)) {
            QMessageBox::warning(&modal, "Stock inválido", "El stock no puede ser negativo.");
            return;
        }

        if (gestor->agregarProducto(nuevo)) {
            gestor->guardarEnArchivo("productos.json");
            modal.accept();
            actualizarVista();
            QMessageBox::information(this, "Éxito", "Producto registrado en el inventario.");
        }
        else {
            QMessageBox::warning(&modal, "No se pudo registrar", "Revisa los datos del producto.");
        }
        });

    modal.exec();
}

// =====================================================
// FILTROS Y BÚSQUEDA
// =====================================================
void VentanaInventario::filtrarPorTexto(const QString& texto) {
    textoBusqueda = texto;
    actualizarVista();
}

void VentanaInventario::setFiltroTodos() {
    soloReposicion = false;
    btnFiltroTodos->setObjectName("btnPestanaActiva");
    btnFiltroReposicion->setObjectName("btnPestanaInactiva");
    estilizarVentana();
    actualizarVista();
}

void VentanaInventario::setFiltroReposicion() {
    soloReposicion = true;
    btnFiltroTodos->setObjectName("btnPestanaInactiva");
    btnFiltroReposicion->setObjectName("btnPestanaActiva");
    estilizarVentana();
    actualizarVista();
}