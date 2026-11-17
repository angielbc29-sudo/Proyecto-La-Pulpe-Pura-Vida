#include "BitacoraView.h"

#include <QComboBox>
#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "Bitacora.h"

// ---------------- Función auxiliar (solo se usa en este archivo) ----------------

namespace
{
    // Devuelve el texto en minúsculas y sin tildes, para que el buscador encuentre "sesión" al escribir "sesion"
    QString sinTildes(const QString& texto)
    {
        QString resultado;
        const QString separado = texto.normalized(QString::NormalizationForm_D);   // separa la letra de su tilde
        for (int i = 0; i < separado.size(); i++)
        {
            if (separado[i].category() != QChar::Mark_NonSpacing)
            {
                resultado += separado[i].toLower();
            }
        }
        return resultado;
    }

    // Crea una celda de texto (sin poder editarla)
    QTableWidgetItem* crearItem(const QString& texto)
    {
        QTableWidgetItem* item = new QTableWidgetItem(texto);
        item->setFlags(Qt::ItemIsEnabled);
        return item;
    }
}

// ---------------- Constructor ----------------

BitacoraView::BitacoraView(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("La Pulpe Pura Vida - Bitácora del sistema");
    setAttribute(Qt::WA_StyledBackground, true);

    // ----- Barra superior -----
    btnVolver = new QPushButton("← Menú Principal", this);
    btnVolver->setObjectName("btnVolver");
    btnVolver->setCursor(Qt::PointingHandCursor);

    QLabel* lblTitulo = new QLabel("Bitácora del Sistema", this);
    lblTitulo->setObjectName("lblTituloBitacora");

    lblCantidad = new QLabel(this);
    lblCantidad->setObjectName("lblCantidad");

    QHBoxLayout* layoutBarra = new QHBoxLayout();
    layoutBarra->setSpacing(16);
    layoutBarra->addWidget(btnVolver);
    layoutBarra->addWidget(lblTitulo);
    layoutBarra->addStretch();
    layoutBarra->addWidget(lblCantidad);

    // ----- Filtros -----
    txtBuscar = new QLineEdit(this);
    txtBuscar->setPlaceholderText("Buscar por usuario, acción o detalle...");
    txtBuscar->setToolTip("Busca por usuario, acción o detalle");
    txtBuscar->setClearButtonEnabled(true);

    cmbUsuario = new QComboBox(this);
    cmbModulo = new QComboBox(this);

    cmbFecha = new QComboBox(this);
    cmbFecha->addItem("Todas las fechas");
    cmbFecha->addItem("Hoy");
    cmbFecha->addItem("Últimos 7 días");

    QHBoxLayout* layoutFiltros = new QHBoxLayout();
    layoutFiltros->setSpacing(12);
    layoutFiltros->addWidget(txtBuscar, 1);
    layoutFiltros->addWidget(cmbUsuario);
    layoutFiltros->addWidget(cmbModulo);
    layoutFiltros->addWidget(cmbFecha);

    // ----- Tabla -----
    tabla = new QTableWidget(this);
    tabla->setColumnCount(6);
    tabla->setHorizontalHeaderLabels({ "Fecha y hora", "Usuario", "Rol", "Módulo", "Acción", "Detalle" });
    tabla->verticalHeader()->setVisible(false);
    tabla->verticalHeader()->setDefaultSectionSize(44);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setSelectionMode(QAbstractItemView::NoSelection);
    tabla->setFocusPolicy(Qt::NoFocus);
    tabla->setShowGrid(false);

    QHeaderView* cabecera = tabla->horizontalHeader();
    cabecera->setSectionResizeMode(QHeaderView::ResizeToContents);
    cabecera->setSectionResizeMode(5, QHeaderView::Stretch);   // el detalle ocupa el espacio que sobre
    cabecera->setHighlightSections(false);

    // El encabezado tiene alto fijo y la franja de la barra de desplazamiento siempre está reservada:
    // así la hoja de estilos puede pintar esa franja igual que el encabezado y redondear las esquinas
    cabecera->setFixedHeight(44);
    tabla->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    for (int columna = 0; columna < 6; columna++)
    {
        tabla->horizontalHeaderItem(columna)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // ----- Ventana completa -----
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(40, 30, 40, 30);
    layoutPrincipal->setSpacing(20);
    layoutPrincipal->addLayout(layoutBarra);
    layoutPrincipal->addLayout(layoutFiltros);
    layoutPrincipal->addWidget(tabla, 1);
    setLayout(layoutPrincipal);

    // Hoja de estilos (colores y formas)
    setStyleSheet(R"(
        BitacoraView { background-color: #fff8ee; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTituloBitacora { font-size: 26px; font-weight: bold; }
        QLabel#lblCantidad { color: #8a7560; }
        QPushButton#btnVolver { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }
        QPushButton#btnVolver:hover { background-color: #fffaf3; border: 1px solid #e67e22; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px 9px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QComboBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 6px 10px; min-height: 22px; min-width: 110px; }
        QComboBox:focus { border: 2px solid #e67e22; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: url(:/ProyectoLaPulpePuraVida/flecha_abajo.png); width: 12px; height: 12px; }
        QComboBox QAbstractItemView { background-color: #ffffff; border: 1px solid #e0c9a6; selection-background-color: #fff3e0; selection-color: #3b2a1a; }
        QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QTableWidget::item { border-bottom: 1px solid #f3e6d0; padding: 6px 16px; }
        QHeaderView { background-color: #fff3e0; border: none; border-top-left-radius: 11px; }
        QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; padding: 0px 16px; }
        QHeaderView::section:first { border-top-left-radius: 11px; }
        QTableWidget QScrollBar:vertical { background: transparent; width: 12px; margin: 44px 0px 12px 0px; border: none; }
        QTableWidget QScrollBar::sub-line:vertical { height: 44px; background-color: #fff3e0; border: none; border-top-right-radius: 11px; subcontrol-position: top; subcontrol-origin: margin; }
        QTableWidget QScrollBar::add-line:vertical { height: 0px; }
        QTableWidget QScrollBar::up-arrow:vertical, QTableWidget QScrollBar::down-arrow:vertical { width: 0px; height: 0px; }
        QTableWidget QScrollBar::handle:vertical { background-color: #e0c9a6; border-radius: 3px; min-height: 30px; margin: 4px 3px 0px 3px; }
        QTableWidget QScrollBar::handle:vertical:hover { background-color: #e67e22; }
        QTableWidget QScrollBar::handle:vertical:disabled { background: transparent; }
        QTableWidget QScrollBar::add-page:vertical, QTableWidget QScrollBar::sub-page:vertical { background: none; }
    )");

    cargarRegistros();

    // Los filtros se conectan después de llenar las listas, para no repetir la carga de la tabla
    connect(btnVolver, &QPushButton::clicked, this, &BitacoraView::volverAlMenu);
    connect(txtBuscar, &QLineEdit::textChanged, this, [this](const QString&) { llenarTabla(); });
    connect(cmbUsuario, &QComboBox::currentIndexChanged, this, [this](int) { llenarTabla(); });
    connect(cmbModulo, &QComboBox::currentIndexChanged, this, [this](int) { llenarTabla(); });
    connect(cmbFecha, &QComboBox::currentIndexChanged, this, [this](int) { llenarTabla(); });

    llenarTabla();
}

// ---------------- Métodos privados ----------------

void BitacoraView::cargarRegistros()
{
    // La bitácora se guarda del más viejo al más nuevo; aquí se quiere lo más reciente arriba
    std::vector<RegistroBitacora> guardados = Bitacora::obtenerTodos();

    registros.clear();
    for (int i = static_cast<int>(guardados.size()) - 1; i >= 0; i--)
    {
        registros.push_back(guardados[i]);
    }

    // Las listas de los filtros se llenan con los usuarios y módulos que aparecen en la bitácora
    cmbUsuario->clear();
    cmbUsuario->addItem("Todos los usuarios");
    cmbModulo->clear();
    cmbModulo->addItem("Todos los módulos");

    for (size_t i = 0; i < registros.size(); i++)
    {
        QString usuario = QString::fromStdString(registros[i].usuario);
        if (cmbUsuario->findText(usuario) == -1)
        {
            cmbUsuario->addItem(usuario);
        }

        QString modulo = QString::fromStdString(registros[i].modulo);
        if (cmbModulo->findText(modulo) == -1)
        {
            cmbModulo->addItem(modulo);
        }
    }
}

void BitacoraView::llenarTabla()
{
    QString busqueda = sinTildes(txtBuscar->text().trimmed());
    QDate hoy = QDate::currentDate();

    tabla->clearSpans();
    tabla->setRowCount(0);

    for (size_t i = 0; i < registros.size(); i++)
    {
        QString fechaHora = QString::fromStdString(registros[i].fechaHora);
        QString usuario = QString::fromStdString(registros[i].usuario);
        QString rol = QString::fromStdString(registros[i].rol);
        QString modulo = QString::fromStdString(registros[i].modulo);
        QString accion = QString::fromStdString(registros[i].accion);
        QString detalle = QString::fromStdString(registros[i].detalle);

        // Filtro por usuario y por módulo (la posición 0 de cada lista es "Todos")
        if (cmbUsuario->currentIndex() > 0 && usuario != cmbUsuario->currentText())
        {
            continue;
        }
        if (cmbModulo->currentIndex() > 0 && modulo != cmbModulo->currentText())
        {
            continue;
        }

        // Filtro por fecha: los primeros 10 caracteres son "yyyy-MM-dd"
        QDate fecha = QDate::fromString(fechaHora.left(10), "yyyy-MM-dd");
        if (cmbFecha->currentIndex() == 1 && fecha != hoy)
        {
            continue;
        }
        if (cmbFecha->currentIndex() == 2 && fecha < hoy.addDays(-6))
        {
            continue;
        }

        // Buscador: el texto puede estar en el usuario, la acción o el detalle
        if (!busqueda.isEmpty()
            && !sinTildes(usuario).contains(busqueda)
            && !sinTildes(accion).contains(busqueda)
            && !sinTildes(detalle).contains(busqueda))
        {
            continue;
        }

        int fila = tabla->rowCount();
        tabla->insertRow(fila);
        tabla->setItem(fila, 0, crearItem(fechaHora));
        tabla->setItem(fila, 1, crearItem(usuario));
        tabla->setItem(fila, 2, crearItem(rol));
        tabla->setItem(fila, 3, crearItem(modulo));
        tabla->setItem(fila, 4, crearItem(accion));
        tabla->setItem(fila, 5, crearItem(detalle));
        tabla->item(fila, 5)->setToolTip(detalle);   // si el detalle es largo, se lee completo al pasar el mouse
    }

    int cantidad = tabla->rowCount();
    lblCantidad->setText(QString::number(cantidad) + (cantidad == 1 ? " registro" : " registros"));

    // Si ningún renglón cumple los filtros, se avisa en un renglón que ocupa toda la tabla
    if (cantidad == 0)
    {
        tabla->insertRow(0);
        tabla->setSpan(0, 0, 1, tabla->columnCount());

        QTableWidgetItem* itemVacio = crearItem("No se encontraron registros con esa búsqueda.");
        itemVacio->setTextAlignment(Qt::AlignCenter);
        itemVacio->setForeground(QBrush(QColor("#8a7560")));
        tabla->setItem(0, 0, itemVacio);
    }
}
