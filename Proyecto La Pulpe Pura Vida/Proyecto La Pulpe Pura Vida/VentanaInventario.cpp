#include "VentanaInventario.h"
#include "Bitacora.h"
#include "Formato.h"
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QScrollBar>
#include <QComboBox>
#include <QRegularExpressionValidator>
#include <QGraphicsDropShadowEffect>



namespace {
    // Devuelve el texto en minúsculas y sin tildes, para que el buscador encuentre "Café" al escribir "cafe"
    QString sinTildes(const QString& texto) {
        QString resultado;
        const QString separado = texto.normalized(QString::NormalizationForm_D);   // separa la letra de su tilde
        for (int i = 0; i < separado.size(); i++) {
            if (separado[i].category() != QChar::Mark_NonSpacing) {
                resultado += separado[i].toLower();
            }
        }
        return resultado;
    }

    // Estilo del fondo de los modales: el selector #fondoModal evita que el borde
    // redondeado se les pegue también a las etiquetas del formulario
    const QString ESTILO_FONDO_MODAL =
        "QWidget#fondoModal { background-color: #fff8ee; border: 1.5px solid #f0d9b5; border-radius: 16px; }"
        "QLabel { color: #7a6650; font-size: 12px; border: none; background: transparent; }";

    // "1 producto" / "3 productos"
    QString textoProductos(int cantidad) {
        return QString::number(cantidad) + (cantidad == 1 ? " producto" : " productos");
    }

    void mostrarAvisoPulpe(QWidget* parent, const QString& titulo, const QString& mensaje, bool esExito = true) {
        QDialog dlg(parent);
        dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        dlg.setAttribute(Qt::WA_TranslucentBackground);

        QWidget* fondo = new QWidget(&dlg);
        fondo->setStyleSheet("background-color: #fff8ee; border: 1.5px solid #f0d9b5; border-radius: 14px;");

        QGraphicsDropShadowEffect* sombra = new QGraphicsDropShadowEffect(&dlg);
        sombra->setBlurRadius(15);
        sombra->setColor(QColor(0, 0, 0, 50));
        fondo->setGraphicsEffect(sombra);

        QVBoxLayout* lPrincipal = new QVBoxLayout(&dlg);
        lPrincipal->setContentsMargins(10, 10, 10, 10);
        lPrincipal->addWidget(fondo);

        QVBoxLayout* v = new QVBoxLayout(fondo);
        v->setContentsMargins(24, 20, 24, 20);
        v->setSpacing(12);

        QLabel* lblT = new QLabel(titulo, fondo);
        lblT->setStyleSheet(QString("font-size: 17px; font-weight: bold; color: %1; border: none;")
            .arg(esExito ? "#1e8449" : "#c0392b"));

        QLabel* lblM = new QLabel(mensaje, fondo);
        lblM->setStyleSheet("font-size: 13px; color: #7a6650; border: none;");
        lblM->setWordWrap(true);

        QPushButton* btnOk = new QPushButton("Entendido", fondo);
        btnOk->setStyleSheet(
            "background-color: #e67e22; color: white; border: none; border-radius: 6px; padding: 8px 20px; font-weight: bold;"
        );
        btnOk->setCursor(Qt::PointingHandCursor);
        QObject::connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);

        QHBoxLayout* hBtn = new QHBoxLayout();
        hBtn->addStretch();
        hBtn->addWidget(btnOk);

        v->addWidget(lblT);
        v->addWidget(lblM);
        v->addLayout(hBtn);

        dlg.setFixedWidth(360);
        dlg.exec();
    }
}


// =====================================================
// CONSTRUCTOR
// =====================================================
VentanaInventario::VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent)
    : QWidget(parent), gestor(gestorCompartido), modoActual(FILTRO_TODOS), textoBusqueda(""),
    lblTotalUnidades(nullptr), lblDetalleUnidades(nullptr), lblTotalAgotados(nullptr),
    lblValorTotal(nullptr), lblTextoAlerta(nullptr), lblResumenPie(nullptr)
{
    setWindowTitle("La Pulpe Pura Vida — Inventario");
    resize(1050, 750);

    configurarInterfaz();
    estilizarVentana();

    if (gestor != nullptr) {
        gestor->cargarDesdeArchivo("productos.json");
        // Si el archivo estaba vacío (no hay ningún producto, ni activo ni inactivo), inicializamos con los del prototipo oficial del informe
        if (gestor->getTodosLosProductos().empty()) {
            gestor->agregarProducto(Producto("001", "Leche Dos Pinos", "Lácteos", 1100.0, 15));
            gestor->agregarProducto(Producto("002", "Arroz Tío Pelón", "Abarrotes", 1400.0, 3));
            gestor->agregarProducto(Producto("003", "Galletas Chiky", "Snacks y galletas", 450.0, 0));
            gestor->agregarProducto(Producto("004", "Café molido", "Abarrotes", 1200.0, 6));
            gestor->agregarProducto(Producto("005", "Agua natural", "Bebidas", 300.0, 20));
            gestor->agregarProducto(Producto("006", "Azúcar blanca", "Abarrotes", 900.0, 10));
            gestor->agregarProducto(Producto("007", "Sandwich de jamón", "Preparados", 1500.0, 10));
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
    lblTitulo->setStyleSheet("font-size: 28px; font-weight: 900; color: #d35400;");

    QLabel* lblSub = new QLabel("Tu mercadería, de un vistazo. Haz clic en cualquier tarjeta para editar o reponer stock.", this);
    lblSub->setStyleSheet("font-size: 13px; color: #7a6650;");

    lTitulos->addWidget(lblTitulo);
    lTitulos->addWidget(lblSub);

    QPushButton* btnVolver = new QPushButton("← Menú Principal", this);
    btnVolver->setObjectName("btnVolver");
    btnVolver->setCursor(Qt::PointingHandCursor);
    connect(btnVolver, &QPushButton::clicked, this, [this]() {
        emit volverAlMenu();
        this->close();
        });

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

    auto crearCard = [](const QString& etiqueta, QLabel*& lblNumero, QLabel*& lblSubtexto, const QString& textoSub, const QString& colorNumero) {
        QFrame* card = new QFrame();
        card->setObjectName("cardMetrica");
        QVBoxLayout* v = new QVBoxLayout(card);
        v->setContentsMargins(18, 14, 18, 14);
        v->setSpacing(2);

        QLabel* lblE = new QLabel(etiqueta, card);
        lblE->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a7560;");

        lblNumero = new QLabel("0", card);
        lblNumero->setStyleSheet("font-size: 28px; font-weight: 900; color: " + colorNumero + ";");

        lblSubtexto = new QLabel(textoSub, card);
        lblSubtexto->setStyleSheet("font-size: 11px; color: #8a7560;");

        v->addWidget(lblE);
        v->addWidget(lblNumero);
        v->addWidget(lblSubtexto);
        return card;
        };

    QLabel* lblAuxAgotados = nullptr;
    QLabel* lblAuxValor = nullptr;

    layoutCards->addWidget(crearCard("UNIDADES DISPONIBLES", lblTotalUnidades, lblDetalleUnidades, "en 0 productos", "#1e8449"));
    layoutCards->addWidget(crearCard("PRODUCTOS AGOTADOS", lblTotalAgotados, lblAuxAgotados, "necesita reposición", "#c0392b"));
    layoutCards->addWidget(crearCard("VALOR DE MERCADERÍA", lblValorTotal, lblAuxValor, "Precio de venta x existencias activas", "#3b2a1a"));
    layoutPrincipal->addLayout(layoutCards);

    // 3. Banner de reposición
    QFrame* banner = new QFrame(this);
    banner->setObjectName("bannerAlerta");
    QHBoxLayout* lBanner = new QHBoxLayout(banner);
    lBanner->setContentsMargins(16, 10, 16, 10);

    lblTextoAlerta = new QLabel(banner);
    lblTextoAlerta->setStyleSheet("font-size: 12px; font-weight: bold; color: #92400e;");
    lblTextoAlerta->setWordWrap(true);   // si hay muchos productos baja de línea en vez de estirar la ventana

    btnVerReposicionBanner = new QPushButton("Ver reposición →", banner);
    btnVerReposicionBanner->setObjectName("btnLinkReposicion");
    btnVerReposicionBanner->setCursor(Qt::PointingHandCursor);
    connect(btnVerReposicionBanner, &QPushButton::clicked, this, &VentanaInventario::setFiltroReposicion);

    lBanner->addWidget(lblTextoAlerta, 1);
    lBanner->addWidget(btnVerReposicionBanner);
    layoutPrincipal->addWidget(banner);

    // 4. Barra de búsqueda y filtros
    QHBoxLayout* lFiltros = new QHBoxLayout();
    txtBuscador = new QLineEdit(this);
    txtBuscador->setPlaceholderText("🔍 Buscar por código o nombre...");
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

    btnFiltroInactivos = new QPushButton("Inactivos", this);
    btnFiltroInactivos->setObjectName("btnPestanaInactiva");
    btnFiltroInactivos->setCursor(Qt::PointingHandCursor);
    connect(btnFiltroInactivos, &QPushButton::clicked, this, &VentanaInventario::setFiltroInactivos);

    lFiltros->addWidget(txtBuscador, 1);
    lFiltros->addSpacing(15);
    lFiltros->addWidget(btnFiltroTodos);
    lFiltros->addWidget(btnFiltroReposicion);
    lFiltros->addWidget(btnFiltroInactivos);
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
            "QScrollBar:vertical { width: 6px; background: transparent; border: none; }"
            "QScrollBar::handle:vertical { background: #e0c9a6; border-radius: 3px; min-height: 30px; }"
            "QScrollBar::handle:vertical:hover { background: #e67e22; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );
    }

    contenedorCards = new QWidget();
    contenedorCards->setStyleSheet("background: transparent;");
    gridProductos = new QGridLayout(contenedorCards);
    gridProductos->setSpacing(16);
    gridProductos->setContentsMargins(0, 8, 0, 8);
    gridProductos->setAlignment(Qt::AlignTop | Qt::AlignLeft);   // las tarjetas empiezan arriba a la izquierda, sin quedar flotando

    scroll->setWidget(contenedorCards);
    layoutPrincipal->addWidget(scroll, 1);

    // 6. Pie de página
    QHBoxLayout* lPie = new QHBoxLayout();
    lblResumenPie = new QLabel("0 productos visibles — 0 unidades en total", this);
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
            background-color: #fff8ee;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QPushButton#btnVolver {
            background-color: #ffffff;
            color: #7a6650;
            border: 1px solid #e0c9a6;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
            padding: 9px 16px;
        }
        QPushButton#btnVolver:hover {
            background-color: #fffaf3;
        }
        QPushButton#btnNuevo {
            background-color: #e67e22;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
            padding: 9px 18px;
        }
        QPushButton#btnNuevo:hover {
            background-color: #cf6d17;
        }
        QFrame#cardMetrica {
            background-color: #ffffff;
            border: 1px solid #f0d9b5;
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
            border: 1px solid #e0c9a6;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 13px;
            color: #3b2a1a;
        }
        QLineEdit#txtBuscador:focus {
            border: 1.5px solid #e67e22;
        }
        QPushButton#btnPestanaActiva {
            background-color: #e67e22;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 7px 16px;
            border: none;
        }
        QPushButton#btnPestanaInactiva {
            background-color: #ffffff;
            color: #7a6650;
            font-weight: bold;
            border-radius: 6px;
            padding: 7px 16px;
            border: 1px solid #e0c9a6;
        }
        QPushButton#btnPestanaInactiva:hover {
            background-color: #fffaf3;
        }
    )");
}

// =====================================================
// CREAR CADA TARJETA INDIVIDUAL
// =====================================================
QWidget* VentanaInventario::crearCardProducto(const Producto& prod) {
    QFrame* card = new QFrame();
    card->setObjectName("cardProducto");
    card->setFixedSize(290, 185);
    card->setCursor(Qt::PointingHandCursor);

    bool esInactivo = !prod.getActivo();
    bool esAgotado = (prod.getStock() == 0 && !esInactivo);
    bool esBajo = (prod.getStock() > 0 && prod.getStock() <= 5 && !esInactivo);

    QString estiloBorde;
    if (esInactivo) {
        estiloBorde = "border: 2px dashed #e0c9a6; background-color: #f9f9f9;";
    }
    else if (esAgotado) {
        estiloBorde = "border: 2px dashed #e6a59c; background-color: #fdecea;";
    }
    else {
        estiloBorde = "border: 1px solid #f0d9b5; background-color: #ffffff;";
    }

    card->setStyleSheet(QString(
        "QFrame#cardProducto {"
        "   %1"
        "   border-radius: 12px;"
        "}"
        "QFrame#cardProducto:hover {"
        "   border: 2px solid #e67e22;"
        "}"
        "QLabel { background: transparent; border: none; font-family: 'Segoe UI'; }"
    ).arg(estiloBorde));

    QVBoxLayout* v = new QVBoxLayout(card);
    v->setContentsMargins(14, 12, 14, 12);
    v->setSpacing(3);

    // Fila superior: Código y botón
    QHBoxLayout* filaTop = new QHBoxLayout();
    QLabel* lblCod = new QLabel(QString::fromStdString(prod.getCodigo()), card);
    lblCod->setStyleSheet("font-size: 11px; font-weight: bold; color: #8a7560;");

    QLabel* lblEditar = new QLabel(esInactivo ? "⚙️ Reactivar" : "✏️ Reponer/Editar", card);
    lblEditar->setStyleSheet(QString("font-size: 11px; font-weight: bold; color: %1;").arg(esInactivo ? "#7a6650" : "#d35400"));

    filaTop->addWidget(lblCod);
    filaTop->addStretch();
    filaTop->addWidget(lblEditar);
    v->addLayout(filaTop);

    // Ilustración simbólica del producto en el centro
    QLabel* lblGrafico = new QLabel(card);
    lblGrafico->setAlignment(Qt::AlignCenter);
    lblGrafico->setFixedHeight(46);
    if (esInactivo) {
        lblGrafico->setText("INACTIVO");
        lblGrafico->setStyleSheet("background-color: #f0f0f0; color: #7a6650; font-weight: bold; font-size: 11px; border: 1px solid #e0c9a6; border-radius: 6px; padding: 4px 10px;");
    }
    else if (esAgotado) {
        lblGrafico->setText("AGOTADO");
        lblGrafico->setStyleSheet("background-color: #fdecea; color: #c0392b; font-weight: bold; font-size: 11px; border: 1px solid #e6a59c; border-radius: 6px; padding: 4px 10px;");
    }
    else {
        QString textoCaja = QString::fromStdString(prod.getNombre()).toUpper().left(10);
        lblGrafico->setText(textoCaja);
        lblGrafico->setStyleSheet("background-color: #fff8ee; color: #3b2a1a; font-weight: bold; font-size: 10px; border: 1px solid #3b2a1a; border-radius: 6px; padding: 4px 12px;");
    }
    v->addWidget(lblGrafico, 0, Qt::AlignCenter);

    // Nombre y categoría
    QLabel* lblNom = new QLabel(QString::fromStdString(prod.getNombre()), card);
    lblNom->setStyleSheet(QString("font-size: 14px; font-weight: bold; color: %1; margin-top: 4px;").arg(esInactivo ? "#8a7560" : "#3b2a1a"));
    lblNom->setWordWrap(true);

    QLabel* lblCat = new QLabel(QString::fromStdString(prod.getCategoria()), card);
    lblCat->setStyleSheet("font-size: 11px; color: #8a7560;");

    v->addWidget(lblNom);
    v->addWidget(lblCat);
    v->addStretch();

    // Fila inferior: Precio y Estado
    QHBoxLayout* filaBot = new QHBoxLayout();
    QLabel* lblPrecio = new QLabel(Formato::colones(prod.getPrecio()), card);   // mismo formato de colones que el resto del sistema
    lblPrecio->setStyleSheet(QString("font-size: 15px; font-weight: 900; color: %1;").arg(esInactivo ? "#8a7560" : "#3b2a1a"));

    QString txtEstado;
    QString colEstado;
    if (esInactivo) {
        txtEstado = "● Desactivado";
        colEstado = "#8a7560";
    }
    else if (esAgotado) {
        txtEstado = "0 unid.  ● Agotado";
        colEstado = "#c0392b";
    }
    else if (esBajo) {
        txtEstado = QString::number(prod.getStock()) + " unid.  ● Stock bajo";
        colEstado = "#d97706";
    }
    else {
        txtEstado = QString::number(prod.getStock()) + " unid.  ● Disponible";
        colEstado = "#1e8449";
    }

    QLabel* lblEstado = new QLabel(txtEstado, card);
    lblEstado->setStyleSheet("font-size: 11px; font-weight: bold; color: " + colEstado + ";");

    filaBot->addWidget(lblPrecio);
    filaBot->addStretch();
    filaBot->addWidget(lblEstado);
    v->addLayout(filaBot);

    // Click en la tarjeta
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
        if (item->widget()) delete item->widget();
        delete item;
    }

    // Calculamos métricas globales de productos activos
    auto productosActivos = gestor->getProductosActivos();
    int totalUnidades = 0;
    int agotados = 0;
    double valorTotal = 0.0;
    QStringList nombresAgotados;
    QStringList nombresBajos;

    for (const auto& prod : productosActivos) {
        totalUnidades += prod.getStock();
        valorTotal += (prod.getPrecio() * prod.getStock());

        if (prod.getStock() == 0) {
            agotados++;
            nombresAgotados.append(QString::fromStdString(prod.getNombre()));
        }
        else if (prod.getStock() <= 5) {
            nombresBajos.append(QString::fromStdString(prod.getNombre()) + " (quedan " + QString::number(prod.getStock()) + ")");
        }
    }

    // Actualizamos las tarjetas superiores
    if (lblTotalUnidades) lblTotalUnidades->setText(QString::number(totalUnidades));
    if (lblDetalleUnidades) lblDetalleUnidades->setText("en " + textoProductos((int)productosActivos.size()));
    if (lblTotalAgotados) lblTotalAgotados->setText(QString("%1").arg(agotados, 2, 10, QChar('0')));
    if (lblValorTotal) lblValorTotal->setText(Formato::colones(valorTotal));

    // Texto de alerta de reposición
    QStringList partesAviso;
    if (!nombresBajos.isEmpty()) partesAviso.append("Por reponer: " + nombresBajos.join(", "));
    if (!nombresAgotados.isEmpty()) partesAviso.append("Agotados: " + nombresAgotados.join(", "));

    QString aviso = "✅ Todas las existencias están en niveles óptimos.";
    if (!partesAviso.isEmpty()) aviso = "⚠️ " + partesAviso.join("  ·  ");
    if (lblTextoAlerta) lblTextoAlerta->setText(aviso);
    if (btnVerReposicionBanner) btnVerReposicionBanner->setVisible(!partesAviso.isEmpty());

    // Los productos se toman del gestor (no se vuelve a leer el archivo desde la vista)
    std::vector<Producto> listaParaMostrar;
    for (const auto& p : gestor->getTodosLosProductos()) {
        if (modoActual == FILTRO_INACTIVOS) {
            if (!p.getActivo()) listaParaMostrar.push_back(p);
        }
        else if (modoActual == FILTRO_REPOSICION) {
            if (p.getActivo() && p.getStock() <= 5) listaParaMostrar.push_back(p);
        }
        else {
            if (p.getActivo()) listaParaMostrar.push_back(p);
        }
    }

    int columna = 0;
    int fila = 0;
    int renderizados = 0;
    QString buscado = sinTildes(textoBusqueda.trimmed());

    for (const auto& prod : listaParaMostrar) {
        if (!buscado.isEmpty()) {
            QString cod = sinTildes(QString::fromStdString(prod.getCodigo()));
            QString nom = sinTildes(QString::fromStdString(prod.getNombre()));
            if (!cod.contains(buscado) && !nom.contains(buscado)) {
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

    // Aviso cuando no hay nada que mostrar (se borra solo en la siguiente actualización, como las tarjetas)
    if (renderizados == 0) {
        QString textoVacio;
        if (!buscado.isEmpty()) textoVacio = "No se encontraron productos que coincidan con la búsqueda.";
        else if (modoActual == FILTRO_INACTIVOS) textoVacio = "No hay productos desactivados.";
        else if (modoActual == FILTRO_REPOSICION) textoVacio = "No hay productos por reponer.";
        else textoVacio = "Todavía no hay productos registrados.";

        QLabel* lblVacio = new QLabel(textoVacio, contenedorCards);
        lblVacio->setStyleSheet("font-size: 13px; color: #8a7560; padding: 20px 4px;");
        gridProductos->addWidget(lblVacio, 0, 0);
    }

    if (lblResumenPie) {
        QString visibles = textoProductos(renderizados) + (renderizados == 1 ? " visible" : " visibles");
        lblResumenPie->setText(visibles + " — " + QString::number(totalUnidades) + " unidades en total");
    }
}

// =====================================================
// MODAL: REPONER O EDITAR PRODUCTO (DISEÑO INTEGRADO)
// =====================================================
void VentanaInventario::abrirModalDetalleProducto(const std::string& codigo) {
    Producto* prod = gestor->buscarPorCodigo(codigo);
    if (prod == nullptr) return;

    QDialog modal(this);
    modal.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    modal.setAttribute(Qt::WA_TranslucentBackground);

    // Contenedor con bordes redondeados y sombra
    QWidget* fondoModal = new QWidget(&modal);
    fondoModal->setObjectName("fondoModal");
    fondoModal->setStyleSheet(ESTILO_FONDO_MODAL);

    QGraphicsDropShadowEffect* sombra = new QGraphicsDropShadowEffect(&modal);
    sombra->setBlurRadius(20);
    sombra->setColor(QColor(0, 0, 0, 60));
    sombra->setOffset(0, 4);
    fondoModal->setGraphicsEffect(sombra);

    QVBoxLayout* lPrincipal = new QVBoxLayout(&modal);
    lPrincipal->setContentsMargins(10, 10, 10, 10);
    lPrincipal->addWidget(fondoModal);

    QVBoxLayout* layout = new QVBoxLayout(fondoModal);
    layout->setContentsMargins(28, 22, 28, 24);
    layout->setSpacing(14);

    // Barra superior con botón X
    QHBoxLayout* lTop = new QHBoxLayout();
    QLabel* titulo = new QLabel("Editar: " + QString::fromStdString(prod->getNombre()), fondoModal);
    titulo->setStyleSheet("font-size: 19px; font-weight: 900; color: #d35400; border: none;");

    QPushButton* btnCerrarX = new QPushButton("✕", fondoModal);
    btnCerrarX->setStyleSheet("border: none; font-size: 16px; font-weight: bold; color: #8a7560; background: transparent;");
    btnCerrarX->setCursor(Qt::PointingHandCursor);
    connect(btnCerrarX, &QPushButton::clicked, &modal, &QDialog::reject);

    lTop->addWidget(titulo);
    lTop->addStretch();
    lTop->addWidget(btnCerrarX);
    layout->addLayout(lTop);

    QLabel* subtitulo = new QLabel("Código: " + QString::fromStdString(prod->getCodigo()) + " (Inmutable) | Categoría: " + QString::fromStdString(prod->getCategoria()), fondoModal);
    subtitulo->setStyleSheet("color: #7a6650; font-size: 12px; border: none; margin-bottom: 4px;");
    layout->addWidget(subtitulo);

    QFormLayout form;

    // Nombre editable (mismas reglas que al crear el producto)
    QLineEdit txtNombre(fondoModal);
    txtNombre.setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9áéíóúÁÉÍÓÚñÑüÜ .-]+$"), &modal));
    txtNombre.setMaxLength(40);
    txtNombre.setText(QString::fromStdString(prod->getNombre()));

    QDoubleSpinBox spinPrecio(fondoModal);
    spinPrecio.setRange(0.0, 1000000.0);
    spinPrecio.setDecimals(0);
    spinPrecio.setValue(prod->getPrecio());   // Empieza con el precio actual del producto
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinSumarStock(fondoModal);
    spinSumarStock.setRange(0, 10000);
    spinSumarStock.setValue(0);
    spinSumarStock.setSuffix(" unid.");

    QLabel* lblStockActual = new QLabel(QString::number(prod->getStock()) + " unidades actualmente en bodega", fondoModal);
    lblStockActual->setStyleSheet("font-weight: bold; color: #1e8449; border: none;");

    QString estiloInput = "background: white; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px; color: #3b2a1a; font-size: 13px;";
    txtNombre.setStyleSheet(estiloInput);
    spinPrecio.setStyleSheet(estiloInput);
    spinSumarStock.setStyleSheet(estiloInput);

    form.addRow("Nombre *:", &txtNombre);
    form.addRow("Stock actual:", lblStockActual);
    form.addRow("Reponer (+ sumar stock):", &spinSumarStock);
    form.addRow("Precio de venta *:", &spinPrecio);
    layout->addLayout(&form);

    QHBoxLayout* lBotones = new QHBoxLayout();

    QPushButton btnBorradoLogico(prod->getActivo() ? "Desactivar Producto" : "Reactivar Producto", fondoModal);
    btnBorradoLogico.setStyleSheet(prod->getActivo()
        ? "background-color: #c0392b; color: white; font-weight: bold; border-radius: 8px; padding: 10px 14px;"
        : "background-color: #1e8449; color: white; font-weight: bold; border-radius: 8px; padding: 10px 14px;");
    btnBorradoLogico.setCursor(Qt::PointingHandCursor);

    QPushButton btnGuardar("Guardar cambios", fondoModal);
    btnGuardar.setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; border-radius: 8px; padding: 10px 18px;");
    btnGuardar.setCursor(Qt::PointingHandCursor);

    lBotones->addWidget(&btnBorradoLogico);
    lBotones->addWidget(&btnGuardar);
    layout->addLayout(lBotones);

    connect(&btnGuardar, &QPushButton::clicked, [&]() {
        std::string nuevoNombre = txtNombre.text().simplified().toStdString();
        double nuevoPrecio = spinPrecio.value();
        int sumar = spinSumarStock.value();

        if (nuevoNombre.empty()) {
            mostrarAvisoPulpe(&modal, "⚠️ Campo obligatorio", "El nombre no puede quedar vacío.", false);
            return;
        }
        if (!gestor->validarNombreUnico(nuevoNombre, codigo)) {
            mostrarAvisoPulpe(&modal, "⚠️ Nombre duplicado", "Ya existe otro producto con ese nombre.", false);
            return;
        }
        if (nuevoPrecio <= 0.0) {
            mostrarAvisoPulpe(&modal, "⚠️ Precio inválido", "El precio debe ser estrictamente mayor a ₡0.", false);
            return;
        }

        // Se guarda lo anterior para saber qué cambió (y anotar solo eso en la bitácora)
        bool cambioNombre = (nuevoNombre != prod->getNombre());
        bool cambioPrecio = (nuevoPrecio != prod->getPrecio());

        gestor->actualizarProducto(codigo, nuevoNombre, nuevoPrecio);
        if (sumar > 0) {
            gestor->buscarPorCodigo(codigo)->agregarStock(sumar);
        }

        if (gestor->guardarEnArchivo("productos.json")) {
            // 📝 REGISTRO EN BITÁCORA:
            if (sumar > 0) {
                Bitacora::registrar("Inventario", "Stock repuesto", "Código " + codigo + ", +" + std::to_string(sumar));
            }
            if (cambioNombre || cambioPrecio) {
                Bitacora::registrar("Inventario", "Producto editado", "Código " + codigo + ", " + nuevoNombre + ", precio ₡" + std::to_string((int)nuevoPrecio));
            }

            actualizarVista();
            mostrarAvisoPulpe(&modal, "✅ Actualizado", "El producto se actualizó y guardó en disco.", true);
            modal.accept();
        }
        else {
            mostrarAvisoPulpe(&modal, "❌ Error", "No se pudo escribir en productos.json.", false);
        }
        });

    connect(&btnBorradoLogico, &QPushButton::clicked, [&]() {
        bool nuevoEstado = !prod->getActivo();

        if (!nuevoEstado) {
            // Cuadro de confirmación estilizado según la paleta oficial
            QDialog dialogoConf(&modal);
            dialogoConf.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            dialogoConf.setAttribute(Qt::WA_TranslucentBackground);

            QWidget* fondoConf = new QWidget(&dialogoConf);
            fondoConf->setStyleSheet("background-color: #fff8ee; border: 1.5px solid #f0d9b5; border-radius: 14px;");

            QGraphicsDropShadowEffect* sombraConf = new QGraphicsDropShadowEffect(&dialogoConf);
            sombraConf->setBlurRadius(15);
            sombraConf->setColor(QColor(0, 0, 0, 50));
            fondoConf->setGraphicsEffect(sombraConf);

            QVBoxLayout* lPrincipalConf = new QVBoxLayout(&dialogoConf);
            lPrincipalConf->setContentsMargins(10, 10, 10, 10);
            lPrincipalConf->addWidget(fondoConf);

            QVBoxLayout* vConf = new QVBoxLayout(fondoConf);
            vConf->setContentsMargins(24, 20, 24, 20);
            vConf->setSpacing(12);

            QLabel* lblTituloConf = new QLabel("¿Desactivar producto?", fondoConf);
            lblTituloConf->setStyleSheet("font-size: 17px; font-weight: bold; color: #c0392b; border: none;");

            QLabel* lblTextoConf = new QLabel("El producto no se borrará, pero no estará disponible en el catálogo de ventas.", fondoConf);
            lblTextoConf->setStyleSheet("font-size: 13px; color: #7a6650; border: none;");
            lblTextoConf->setWordWrap(true);

            QHBoxLayout* hBtnsConf = new QHBoxLayout();
            QPushButton* btnNo = new QPushButton("Cancelar", fondoConf);
            btnNo->setStyleSheet("background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 6px; padding: 8px 16px; font-weight: bold;");
            btnNo->setCursor(Qt::PointingHandCursor);

            QPushButton* btnSi = new QPushButton("Sí, desactivar", fondoConf);
            btnSi->setStyleSheet("background-color: #c0392b; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold;");
            btnSi->setCursor(Qt::PointingHandCursor);

            hBtnsConf->addWidget(btnNo);
            hBtnsConf->addWidget(btnSi);

            vConf->addWidget(lblTituloConf);
            vConf->addWidget(lblTextoConf);
            vConf->addLayout(hBtnsConf);

            connect(btnNo, &QPushButton::clicked, &dialogoConf, &QDialog::reject);
            connect(btnSi, &QPushButton::clicked, &dialogoConf, &QDialog::accept);

            dialogoConf.setFixedWidth(360);
            if (dialogoConf.exec() != QDialog::Accepted) {
                return;
            }
        }

        gestor->cambiarEstado(codigo, nuevoEstado);
        if (!gestor->guardarEnArchivo("productos.json")) {
            gestor->cambiarEstado(codigo, !nuevoEstado);   // se deja como estaba
            mostrarAvisoPulpe(&modal, "❌ Error", "No se pudo escribir en productos.json.", false);
            return;
        }
        // 📝 REGISTRO EN BITÁCORA:
        if (nuevoEstado) {
            Bitacora::registrar("Inventario", "Producto reactivado", "Código " + codigo);
        }
        else {
            Bitacora::registrar("Inventario", "Producto desactivado", "Código " + codigo);
        }

        actualizarVista();
        modal.accept();
        });

    modal.setFixedWidth(440);
    modal.exec();
}

// =====================================================
// MODAL NUEVO PRODUCTO (DISEÑO INTEGRADO Y BLOQUEO DE TECLADO)
// =====================================================
void VentanaInventario::abrirModalNuevoProducto() {
    QDialog modal(this);
    modal.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    modal.setAttribute(Qt::WA_TranslucentBackground);

    QWidget* fondoModal = new QWidget(&modal);
    fondoModal->setObjectName("fondoModal");
    fondoModal->setStyleSheet(ESTILO_FONDO_MODAL);

    QGraphicsDropShadowEffect* sombra = new QGraphicsDropShadowEffect(&modal);
    sombra->setBlurRadius(20);
    sombra->setColor(QColor(0, 0, 0, 60));
    sombra->setOffset(0, 4);
    fondoModal->setGraphicsEffect(sombra);

    QVBoxLayout* lPrincipal = new QVBoxLayout(&modal);
    lPrincipal->setContentsMargins(10, 10, 10, 10);
    lPrincipal->addWidget(fondoModal);

    QVBoxLayout* layout = new QVBoxLayout(fondoModal);
    layout->setContentsMargins(28, 22, 28, 24);
    layout->setSpacing(14);

    QHBoxLayout* lTop = new QHBoxLayout();
    QLabel* titulo = new QLabel("Nuevo producto", fondoModal);
    titulo->setStyleSheet("font-size: 20px; font-weight: 900; color: #d35400; border: none;");

    QPushButton* btnCerrarX = new QPushButton("✕", fondoModal);
    btnCerrarX->setStyleSheet("border: none; font-size: 16px; font-weight: bold; color: #8a7560; background: transparent;");
    btnCerrarX->setCursor(Qt::PointingHandCursor);
    connect(btnCerrarX, &QPushButton::clicked, &modal, &QDialog::reject);

    lTop->addWidget(titulo);
    lTop->addStretch();
    lTop->addWidget(btnCerrarX);
    layout->addLayout(lTop);

    QLabel* subtitulo = new QLabel("Ingresa los datos del artículo para sumarlo a la bodega.", fondoModal);
    subtitulo->setStyleSheet("color: #7a6650; font-size: 12px; border: none; margin-bottom: 4px;");
    layout->addWidget(subtitulo);

    QFormLayout form;
    QLineEdit txtCodigo(fondoModal);
    QLineEdit txtNombre(fondoModal);
    QComboBox cmbCategoria(fondoModal);
    cmbCategoria.addItems({ "Abarrotes", "Lácteos", "Snacks y galletas", "Bebidas", "Frescos", "Preparados" });

    // 🔒 1. BLOQUEO ESTRICTO: CÓDIGO SOLO NÚMEROS (0-9)
    QRegularExpression rxCodigo("^[0-9]{1,8}$");
    txtCodigo.setValidator(new QRegularExpressionValidator(rxCodigo, &modal));
    txtCodigo.setPlaceholderText("Ej: 008 (solo números)");

    // 🔒 2. BLOQUEO ESTRICTO: NOMBRE SOLO LETRAS, NÚMEROS, ESPACIOS, PUNTO Y GUION (máximo 40 caracteres)
    QRegularExpression rxNombre("^[a-zA-Z0-9áéíóúÁÉÍÓÚñÑüÜ .-]+$");
    txtNombre.setValidator(new QRegularExpressionValidator(rxNombre, &modal));
    txtNombre.setMaxLength(40);
    txtNombre.setPlaceholderText("Ej: Natilla La Granja 400g");

    // 🔒 3. PRECIO Y STOCK: SpinBoxes bloquean físicamente cualquier letra
    QDoubleSpinBox spinPrecio(fondoModal);
    spinPrecio.setRange(0.0, 1000000.0);
    spinPrecio.setDecimals(0);
    spinPrecio.setValue(1000.0);
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinStock(fondoModal);
    spinStock.setRange(0, 10000);
    spinStock.setValue(0);                 // Empieza en 0
    spinStock.setSuffix(" unid.");
    spinStock.setSpecialValueText("0 unid."); // Si se borra, se queda en 0 unid.

    QString estiloInput = "background: white; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px; color: #3b2a1a; font-size: 13px;";
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

    QPushButton btnAceptar("Guardar producto", fondoModal);
    btnAceptar.setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; border-radius: 8px; padding: 11px; margin-top: 10px; font-size: 14px;");
    btnAceptar.setCursor(Qt::PointingHandCursor);
    layout->addWidget(&btnAceptar);

    connect(&btnAceptar, &QPushButton::clicked, [&]() {
        std::string cod = txtCodigo.text().trimmed().toStdString();
        std::string nom = txtNombre.text().simplified().toStdString();   // quita espacios de más
        std::string cat = cmbCategoria.currentText().toStdString();



        double precio = spinPrecio.value();
        int stock = spinStock.value();

        // 1. Campos obligatorios
        if (cod.empty() || nom.empty()) {
            mostrarAvisoPulpe(&modal, "⚠️ Campos obligatorios", "El código y el nombre no pueden quedar vacíos.", false);
            return;
        }

        // 2. Validación de código único
        if (!gestor->validarCodigoUnico(cod)) {
            mostrarAvisoPulpe(&modal, "⚠️ Código duplicado", "Ya existe un producto registrado con ese código único.", false);
            return;
        }

        // 2.1 Validación de nombre único
        if (!gestor->validarNombreUnico(nom)) {
            mostrarAvisoPulpe(&modal, "⚠️ Nombre duplicado", "Ya existe un producto con ese nombre.", false);
            return;
        }

        // 3. Validación de precio
        if (precio <= 0.0) {
            mostrarAvisoPulpe(&modal, "⚠️ Precio inválido", "El precio debe ser estrictamente mayor a ₡0.", false);
            return;
        }

        // 4. Validación de stock
        if (stock < 0) {
            mostrarAvisoPulpe(&modal, "⚠️ Stock inválido", "El stock inicial no puede ser negativo.", false);
            return;
        }

        Producto nuevo(cod, nom, cat, precio, stock);

        if (gestor->agregarProducto(nuevo)) {
            if (!gestor->guardarEnArchivo("productos.json")) {
                mostrarAvisoPulpe(&modal, "❌ Error", "No se pudo escribir en productos.json.", false);
                return;
            }
            // 📝 REGISTRO EN BITÁCORA:
            Bitacora::registrar("Inventario", "Producto creado", "Código " + cod + ", " + nom);
            actualizarVista();
            mostrarAvisoPulpe(&modal, "✅ Producto Registrado", "El producto se sumó a la bodega con éxito.", true);
            modal.accept();
        }
        else {
            mostrarAvisoPulpe(&modal, "❌ Error", "No se pudo registrar el producto.", false);
        }
        });

    modal.setFixedWidth(440);
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
    modoActual = FILTRO_TODOS;
    btnFiltroTodos->setObjectName("btnPestanaActiva");
    btnFiltroReposicion->setObjectName("btnPestanaInactiva");
    btnFiltroInactivos->setObjectName("btnPestanaInactiva");
    estilizarVentana();
    actualizarVista();
}

void VentanaInventario::setFiltroReposicion() {
    modoActual = FILTRO_REPOSICION;
    btnFiltroTodos->setObjectName("btnPestanaInactiva");
    btnFiltroReposicion->setObjectName("btnPestanaActiva");
    btnFiltroInactivos->setObjectName("btnPestanaInactiva");
    estilizarVentana();
    actualizarVista();
}

void VentanaInventario::setFiltroInactivos() {
    modoActual = FILTRO_INACTIVOS;
    btnFiltroTodos->setObjectName("btnPestanaInactiva");
    btnFiltroReposicion->setObjectName("btnPestanaInactiva");
    btnFiltroInactivos->setObjectName("btnPestanaActiva");
    estilizarVentana();
    actualizarVista();
}