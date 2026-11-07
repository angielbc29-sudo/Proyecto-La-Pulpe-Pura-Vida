#include "VentanaInventario.h"
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>

// =====================================================
// CONSTRUCTOR
// =====================================================
VentanaInventario::VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent)
    : QWidget(parent), gestor(gestorCompartido)
{
    setWindowTitle("La Pulpe Pura Vida — Estante Vivo");
    resize(1100, 720);

    configurarInterfaz();
    estilizarVentana();

    if (gestor != nullptr) {
        gestor->cargarDesdeArchivo("productos.json");
        if (gestor->getProductosActivos().empty()) {
            gestor->agregarProducto(Producto("001", "Leche Dos Pinos", "Lacteos", 1100.0, 15));
            gestor->agregarProducto(Producto("002", "Arroz Tio Pelon", "Abarrotes", 1400.0, 3));
            gestor->agregarProducto(Producto("003", "Galletas Chiky", "Snacks", 450.0, 0));
            gestor->agregarProducto(Producto("004", "Cafe molido", "Abarrotes", 1200.0, 6));
            gestor->agregarProducto(Producto("005", "Agua natural", "Bebidas", 300.0, 20));
            gestor->agregarProducto(Producto("006", "Azucar blanca", "Abarrotes", 900.0, 10));
            gestor->agregarProducto(Producto("007", "Sandwich de jamon", "Preparados", 1500.0, 10));
            gestor->guardarEnArchivo("productos.json");
        }
    }

    actualizarVista();
}

// =====================================================
// CONFIGURACIÓN DE LA INTERFAZ
// =====================================================
void VentanaInventario::configurarInterfaz() {
    QHBoxLayout* layoutGeneral = new QHBoxLayout(this);
    layoutGeneral->setContentsMargins(0, 0, 0, 0);
    layoutGeneral->setSpacing(0);

    // =====================================================
    // 1. BARRA LATERAL IZQUIERDA (Vino tinto pulpero #872017)
    // =====================================================
    QFrame* sidebar = new QFrame(this);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(230);
    QVBoxLayout* layoutSidebar = new QVBoxLayout(sidebar);
    layoutSidebar->setContentsMargins(20, 30, 20, 25);
    layoutSidebar->setSpacing(15);

    // Logo Pulpería
    QLabel* lblLogoIcon = new QLabel("🏪", sidebar);
    lblLogoIcon->setStyleSheet("font-size: 38px; background: transparent;");

    QLabel* lblLogoTitulo = new QLabel("La pulpe", sidebar);
    lblLogoTitulo->setStyleSheet("font-size: 24px; font-weight: 900; color: #ffffff; background: transparent;");

    QLabel* lblLogoSub = new QLabel("pura vida", sidebar);
    lblLogoSub->setStyleSheet("font-size: 13px; font-weight: 600; color: #f5c49b; background: transparent; margin-bottom: 25px;");

    layoutSidebar->addWidget(lblLogoIcon);
    layoutSidebar->addWidget(lblLogoTitulo);
    layoutSidebar->addWidget(lblLogoSub);

    // Botón Pestaña Activa: Inventario
    QPushButton* btnTabInventario = new QPushButton("📖  Inventario", sidebar);
    btnTabInventario->setObjectName("btnTabActivo");
    btnTabInventario->setCursor(Qt::PointingHandCursor);
    layoutSidebar->addWidget(btnTabInventario);

    // Botón Volver al Menú Principal
    QPushButton* btnVolver = new QPushButton("←  Volver al Menú", sidebar);
    btnVolver->setObjectName("btnVolver");
    btnVolver->setCursor(Qt::PointingHandCursor);
    connect(btnVolver, &QPushButton::clicked, this, &QWidget::close);
    layoutSidebar->addWidget(btnVolver);

    layoutSidebar->addStretch();

    // Pie de la barra lateral
    QLabel* lblPieSidebar = new QLabel("LP / COSTA RICA\nTodo en su lugar.", sidebar);
    lblPieSidebar->setStyleSheet("font-size: 11px; color: #f5c49b; opacity: 0.8; background: transparent; line-height: 1.4;");
    layoutSidebar->addWidget(lblPieSidebar);

    layoutGeneral->addWidget(sidebar);

    // =====================================================
    // 2. PANEL PRINCIPAL DERECHO (Beige cálido #fff9f2)
    // =====================================================
    QWidget* panelPrincipal = new QWidget(this);
    panelPrincipal->setObjectName("panelPrincipal");
    QVBoxLayout* layoutContenido = new QVBoxLayout(panelPrincipal);
    layoutContenido->setContentsMargins(35, 25, 35, 25);
    layoutContenido->setSpacing(14);

    // Encabezado
    QLabel* lblTag = new QLabel("ESTANTE VIVO", panelPrincipal);
    lblTag->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a6a4f; letter-spacing: 1px;");

    QHBoxLayout* lTitulos = new QHBoxLayout();
    QLabel* lblTitulo = new QLabel("Una mirada diferente a tu inventario", panelPrincipal);
    lblTitulo->setStyleSheet("font-size: 24px; font-weight: 900; color: #3b2a1a;");

    QPushButton* btnNuevo = new QPushButton("+ Nuevo Producto", panelPrincipal);
    btnNuevo->setObjectName("btnNuevoProducto");
    btnNuevo->setCursor(Qt::PointingHandCursor);
    connect(btnNuevo, &QPushButton::clicked, this, &VentanaInventario::abrirModalNuevoProducto);

    lTitulos->addWidget(lblTitulo);
    lTitulos->addStretch();
    lTitulos->addWidget(btnNuevo);

    layoutContenido->addWidget(lblTag);
    layoutContenido->addLayout(lTitulos);

    // =====================================================
    // 3. TARJETAS DE MÉTRICAS (Igual al prototipo)
    // =====================================================
    QHBoxLayout* layoutMetricas = new QHBoxLayout();
    layoutMetricas->setSpacing(14);

    auto crearCardMetrica = [](const QString& etiqueta, QLabel*& lblNumero, const QString& detalle, const QString& colorNum) {
        QFrame* f = new QFrame();
        f->setObjectName("cardMetrica");
        QVBoxLayout* v = new QVBoxLayout(f);
        v->setContentsMargins(16, 12, 16, 12);
        v->setSpacing(2);

        QLabel* lblE = new QLabel(etiqueta, f);
        lblE->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a6a4f;");

        lblNumero = new QLabel("0", f);
        lblNumero->setStyleSheet("font-size: 28px; font-weight: 900; color: " + colorNum + ";");

        QLabel* lblD = new QLabel(detalle, f);
        lblD->setStyleSheet("font-size: 11px; color: #8a7560;");

        v->addWidget(lblE);
        v->addWidget(lblNumero);
        v->addWidget(lblD);
        return f;
        };

    layoutMetricas->addWidget(crearCardMetrica("UNIDADES DISPONIBLES", lblTotalUnidades, "en bodega activa", "#16a34a"));
    layoutMetricas->addWidget(crearCardMetrica("PRODUCTOS AGOTADOS", lblTotalAgotados, "necesitan reposición", "#dc2626"));
    layoutMetricas->addWidget(crearCardMetrica("VALOR DE MERCADERÍA", lblValorTotal, "precio de venta × stock", "#3b2a1a"));
    layoutContenido->addLayout(layoutMetricas);

    // =====================================================
    // 4. BANNER DE ALERTA DE REPOSICIÓN
    // =====================================================
    QFrame* banner = new QFrame(panelPrincipal);
    banner->setObjectName("bannerAlerta");
    QHBoxLayout* lBanner = new QHBoxLayout(banner);
    lBanner->setContentsMargins(14, 8, 14, 8);

    lblTextoAlerta = new QLabel(banner);
    lblTextoAlerta->setStyleSheet("font-size: 12px; font-weight: bold; color: #991b1b;");
    lBanner->addWidget(lblTextoAlerta);
    lBanner->addStretch();

    layoutContenido->addWidget(banner);

    // =====================================================
    // 5. CUADRÍCULA CON SCROLL PARA EL ESTANTE VIVO
    // =====================================================
    QScrollArea* scroll = new QScrollArea(panelPrincipal);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #d4b89b; border-radius: 3px; }"
    );

    contenedorCards = new QWidget();
    contenedorCards->setStyleSheet("background: transparent;");
    gridProductos = new QGridLayout(contenedorCards);
    gridProductos->setSpacing(16);
    gridProductos->setContentsMargins(0, 10, 0, 10);

    scroll->setWidget(contenedorCards);
    layoutContenido->addWidget(scroll, 1);

    layoutGeneral->addWidget(panelPrincipal, 1);
}

// =====================================================
// ESTILOS VISUALES GENERALES
// =====================================================
void VentanaInventario::estilizarVentana() {
    setStyleSheet(R"(
        VentanaInventario {
            background-color: #fcf7f0;
        }
        QFrame#sidebar {
            background-color: #872017;
        }
        QPushButton#btnTabActivo {
            background-color: #fff9f2;
            color: #872017;
            font-size: 14px;
            font-weight: bold;
            border-radius: 10px;
            padding: 12px 18px;
            text-align: left;
            border: none;
        }
        QPushButton#btnVolver {
            background-color: transparent;
            color: #ffffff;
            font-size: 13px;
            font-weight: bold;
            border: 1px solid rgba(255,255,255,0.4);
            border-radius: 10px;
            padding: 10px 18px;
            text-align: left;
            margin-top: 10px;
        }
        QPushButton#btnVolver:hover {
            background-color: rgba(255,255,255,0.15);
        }
        QWidget#panelPrincipal {
            background-color: #fcf7f0;
        }
        QFrame#cardMetrica {
            background-color: #ffffff;
            border: 1px solid #ebd9c5;
            border-radius: 12px;
        }
        QFrame#bannerAlerta {
            background-color: #fde8e8;
            border: 1px solid #f8b4b4;
            border-radius: 10px;
        }
        QPushButton#btnNuevoProducto {
            background-color: #872017;
            color: white;
            font-size: 13px;
            font-weight: bold;
            padding: 10px 20px;
            border-radius: 8px;
            border: none;
        }
        QPushButton#btnNuevoProducto:hover {
            background-color: #a3271c;
        }
    )");
}

// =====================================================
// CREAR LA TARJETA VISUAL DE CADA PRODUCTO ("Estante Vivo")
// =====================================================
QWidget* VentanaInventario::crearCardProducto(const Producto& prod) {
    QFrame* card = new QFrame();
    card->setFixedSize(225, 155);

    bool esAgotado = (prod.getStock() == 0);
    bool esBajo = (prod.getStock() > 0 && prod.getStock() <= 5);

    QString estiloBorde = esAgotado ? "border: 2px dashed #e17055; background-color: #fde8e4;"
        : "border: 1px solid #ebd2b9; background-color: #faebd7;";

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
    v->setSpacing(4);

    // Fila superior: Codigo y Categoria
    QHBoxLayout* filaTop = new QHBoxLayout();
    QLabel* lblCod = new QLabel(QString::fromStdString(prod.getCodigo()), card);
    lblCod->setStyleSheet("font-size: 11px; font-weight: bold; color: #7f5539;");

    QLabel* lblCat = new QLabel(QString::fromStdString(prod.getCategoria()), card);
    lblCat->setStyleSheet("font-size: 11px; color: #8d6b4f;");

    filaTop->addWidget(lblCod);
    filaTop->addStretch();
    filaTop->addWidget(lblCat);
    v->addLayout(filaTop);

    // Nombre del producto (GRANDE Y NÍTIDO)
    QLabel* lblNom = new QLabel(QString::fromStdString(prod.getNombre()), card);
    lblNom->setStyleSheet("font-size: 14px; font-weight: bold; color: #2b1810;");
    lblNom->setWordWrap(true);
    v->addWidget(lblNom);

    // Precio en colones
    QLocale loc(QLocale::Spanish, QLocale::CostaRica);
    QString precioFormateado = "₡ " + loc.toString((int)prod.getPrecio());
    QLabel* lblPrecio = new QLabel(precioFormateado, card);
    lblPrecio->setStyleSheet("font-size: 15px; font-weight: 900; color: #872017; margin-top: 2px;");
    v->addWidget(lblPrecio);

    v->addStretch();

    // Fila inferior: Existencias y Estado
    QHBoxLayout* filaBot = new QHBoxLayout();
    QString textoStock = QString::number(prod.getStock()) + " unid.";
    QString textoEstado = esAgotado ? "● Agotado" : (esBajo ? "● Stock bajo" : "● Disponible");
    QString colorEstado = esAgotado ? "#c0392b" : (esBajo ? "#d35400" : "#27ae60");

    QLabel* lblStock = new QLabel(textoStock, card);
    lblStock->setStyleSheet("font-size: 12px; font-weight: bold; color: #2b1810;");

    QLabel* lblPunto = new QLabel(textoEstado, card);
    lblPunto->setStyleSheet("font-size: 11px; font-weight: bold; color: " + colorEstado + ";");

    filaBot->addWidget(lblStock);
    filaBot->addStretch();
    filaBot->addWidget(lblPunto);
    v->addLayout(filaBot);

    return card;
}

// =====================================================
// ACTUALIZAR ESTANTE VIVO Y MÉTRICAS
// =====================================================
void VentanaInventario::actualizarVista() {
    if (gestor == nullptr) return;

    // Limpiar cuadrícula anterior
    QLayoutItem* item;
    while ((item = gridProductos->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
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
    int columnasMaximas = 3; // 3 tarjetas por fila como en el prototipo

    for (const auto& prod : productos) {
        totalUnidades += prod.getStock();
        valorTotal += (prod.getPrecio() * prod.getStock());

        if (prod.getStock() == 0) {
            agotados++;
            nombresAgotados.append(QString::fromStdString(prod.getNombre()));
        }
        else if (prod.getStock() <= 5) {
            nombresBajos.append(QString::fromStdString(prod.getNombre()) + " (" + QString::number(prod.getStock()) + ")");
        }

        QWidget* card = crearCardProducto(prod);
        gridProductos->addWidget(card, fila, columna);

        columna++;
        if (columna >= columnasMaximas) {
            columna = 0;
            fila++;
        }
    }

    // Actualizar números de las métricas
    QLocale loc(QLocale::Spanish, QLocale::CostaRica);
    lblTotalUnidades->setText(QString::number(totalUnidades) + " unid.");
    lblTotalAgotados->setText(QString("%1").arg(agotados, 2, 10, QChar('0')));
    lblValorTotal->setText("₡ " + loc.toString((int)valorTotal));

    // Texto de la alerta
    QString aviso = "⚠️ Por reponer... ";
    if (!nombresBajos.isEmpty()) aviso += "Poco stock: " + nombresBajos.join(", ") + ". ";
    if (!nombresAgotados.isEmpty()) aviso += "Agotados: " + nombresAgotados.join(", ") + ".";
    if (nombresBajos.isEmpty() && nombresAgotados.isEmpty()) aviso = "✅ Todas las existencias están en niveles óptimos.";
    lblTextoAlerta->setText(aviso);
}

// =====================================================
// MODAL NUEVO PRODUCTO
// =====================================================
void VentanaInventario::abrirModalNuevoProducto() {
    QDialog modal(this);
    modal.setWindowTitle("Nuevo Producto — La Pulpe");
    modal.setFixedWidth(420);
    modal.setStyleSheet("background-color: #fff9f2; font-family: 'Segoe UI'; font-size: 13px; color: #3b2a1a;");

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
    QLineEdit txtCategoria(&modal);

    QDoubleSpinBox spinPrecio(&modal);
    spinPrecio.setRange(1.0, 1000000.0);
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinStock(&modal);
    spinStock.setRange(0, 10000);
    spinStock.setSuffix(" unid.");

    QString estiloInput = "background: white; border: 1px solid #ebd9c5; border-radius: 6px; padding: 7px;";
    txtCodigo.setStyleSheet(estiloInput);
    txtNombre.setStyleSheet(estiloInput);
    txtCategoria.setStyleSheet(estiloInput);
    spinPrecio.setStyleSheet(estiloInput);
    spinStock.setStyleSheet(estiloInput);

    form.addRow("CÓDIGO ÚNICO *:", &txtCodigo);
    form.addRow("NOMBRE DEL PRODUCTO *:", &txtNombre);
    form.addRow("CATEGORÍA *:", &txtCategoria);
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
        std::string cat = txtCategoria.text().trimmed().toStdString();
        double precio = spinPrecio.value();
        int stock = spinStock.value();

        if (cod.empty() || nom.empty()) {
            QMessageBox::warning(&modal, "Error", "El código y el nombre son obligatorios.");
            return;
        }

        if (!gestor->validarCodigoUnico(cod)) {
            QMessageBox::warning(&modal, "Código repetido", "Ya existe un artículo con ese código.");
            return;
        }

        Producto nuevo(cod, nom, cat, precio, stock);
        if (gestor->agregarProducto(nuevo)) {
            gestor->guardarEnArchivo("productos.json");
            modal.accept();
            actualizarVista();
        }
        });

    modal.exec();
}

// =====================================================
// GUARDAR EN DISCO
// =====================================================
void VentanaInventario::guardarCambiosEnDisco() {
    if (gestor != nullptr && gestor->guardarEnArchivo("productos.json")) {
        QMessageBox::information(this, "Guardado exitoso", "Los productos se guardaron correctamente en productos.json.");
    }
}