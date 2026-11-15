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
        QComboBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 6px 10px; min-height: 22px; min-width: 150px; }
        QComboBox:focus { border: 2px solid #e67e22; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: url(:/ProyectoLaPulpePuraVida/flecha_abajo.png); width: 12px; height: 12px; }
        QComboBox QAbstractItemView { background-color: #ffffff; border: 1px solid #e0c9a6; selection-background-color: #fff3e0; selection-color: #3b2a1a; }
        QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QTableWidget::item { border-bottom: 1px solid #f3e6d0; padding: 6px; }
        QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; border-bottom: 1px solid #f0d9b5; padding: 10px; }
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
    QString busqueda = txtBuscar->text().trimmed();
    QDate hoy = QDate::currentDate();

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
            && !usuario.contains(busqueda, Qt::CaseInsensitive)
            && !accion.contains(busqueda, Qt::CaseInsensitive)
            && !detalle.contains(busqueda, Qt::CaseInsensitive))
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
    }

    int cantidad = tabla->rowCount();
    lblCantidad->setText(QString::number(cantidad) + (cantidad == 1 ? " registro" : " registros"));
}
