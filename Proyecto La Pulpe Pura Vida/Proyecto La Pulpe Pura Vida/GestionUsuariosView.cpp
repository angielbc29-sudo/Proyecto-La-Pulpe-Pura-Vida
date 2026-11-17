#include "GestionUsuariosView.h"

#include <memory>
#include <vector>

#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include "Bitacora.h"
#include "EditarUsuarioDialog.h"
#include "IUsuarioRepository.h"
#include "RegistroUsuarioDialog.h"
#include "Sesion.h"
#include "Usuario.h"

// ---------------- Función auxiliar (solo se usa en este archivo) ----------------

namespace
{
    // Devuelve el texto en minúsculas y sin tildes, para que el buscador encuentre "María" al escribir "maria"
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
    QTableWidgetItem* crearItem(const QString& texto, bool centrado)
    {
        QTableWidgetItem* item = new QTableWidgetItem(texto);
        item->setFlags(Qt::ItemIsEnabled);
        if (centrado)
        {
            item->setTextAlignment(Qt::AlignCenter);
        }
        return item;
    }
}

// ---------------- Constructor ----------------

GestionUsuariosView::GestionUsuariosView(IUsuarioRepository* repo, QWidget* parent)
    : QWidget(parent)
{
    this->repo = repo;
    this->idPorDesactivar = -1;
    this->fondoFormulario = nullptr;

    setWindowTitle("La Pulpe Pura Vida - Gestión de usuarios y permisos");
    setAttribute(Qt::WA_StyledBackground, true);

    // ----- Barra superior -----
    btnVolver = new QPushButton("← Menú Principal", this);
    btnVolver->setObjectName("btnVolver");
    btnVolver->setCursor(Qt::PointingHandCursor);

    QLabel* lblTitulo = new QLabel("Gestión de Usuarios y Permisos", this);
    lblTitulo->setObjectName("lblTituloGestion");

    btnRegistrar = new QPushButton("+ Registrar Nuevo Usuario", this);
    btnRegistrar->setObjectName("btnRegistrar");
    btnRegistrar->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutBarra = new QHBoxLayout();
    layoutBarra->setSpacing(16);
    layoutBarra->addWidget(btnVolver);
    layoutBarra->addWidget(lblTitulo);
    layoutBarra->addStretch();
    layoutBarra->addWidget(btnRegistrar);

    // ----- Filtros -----
    txtBuscar = new QLineEdit(this);
    txtBuscar->setPlaceholderText("Buscar por usuario, nombre, cédula o teléfono...");
    txtBuscar->setClearButtonEnabled(true);

    cmbRol = new QComboBox(this);
    cmbRol->addItem("Todos los roles");
    cmbRol->addItem("Cajero");
    cmbRol->addItem("Administrador");
    cmbRol->addItem("Auditor");

    cmbEstado = new QComboBox(this);
    cmbEstado->addItem("Todos los estados");
    cmbEstado->addItem("Activos");
    cmbEstado->addItem("Inactivos");

    QHBoxLayout* layoutFiltros = new QHBoxLayout();
    layoutFiltros->setSpacing(12);
    layoutFiltros->addWidget(txtBuscar, 1);
    layoutFiltros->addWidget(cmbRol);
    layoutFiltros->addWidget(cmbEstado);

    // ----- Tabla -----
    tabla = new QTableWidget(this);
    tabla->setColumnCount(9);
    tabla->setHorizontalHeaderLabels({ "N.º", "Usuario", "Nombre Completo", "Cédula",
                                       "Teléfono", "Rol", "Turno", "Estado", "Acciones" });
    tabla->verticalHeader()->setVisible(false);
    tabla->verticalHeader()->setDefaultSectionSize(56);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setSelectionMode(QAbstractItemView::NoSelection);
    tabla->setFocusPolicy(Qt::NoFocus);
    tabla->setShowGrid(false);
    tabla->setWordWrap(false);   // el nombre va en un solo renglón, igual en ventana pequeña que en pantalla completa

    QHeaderView* cabecera = tabla->horizontalHeader();
    cabecera->setSectionResizeMode(QHeaderView::ResizeToContents);
    cabecera->setSectionResizeMode(2, QHeaderView::Stretch);   // el nombre ocupa el espacio que sobre
    cabecera->setSectionResizeMode(8, QHeaderView::Fixed);     // la columna de los botones tiene un ancho fijo
    tabla->setColumnWidth(8, 212);
    cabecera->setHighlightSections(false);

    // El encabezado tiene alto fijo y la franja de la barra de desplazamiento siempre está reservada:
    // así la hoja de estilos puede pintar esa franja igual que el encabezado y redondear las esquinas
    cabecera->setFixedHeight(44);
    tabla->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Los títulos de las columnas de texto van a la izquierda, igual que su contenido
    for (int columna = 1; columna <= 6; columna++)
    {
        tabla->horizontalHeaderItem(columna)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // ----- Aviso (empieza oculto) -----
    aviso = new QFrame(this);
    aviso->setObjectName("aviso");

    lblAviso = new QLabel(aviso);
    lblAviso->setObjectName("lblAviso");
    lblAviso->setWordWrap(true);

    btnAvisoCancelar = new QPushButton("Cancelar", aviso);
    btnAvisoCancelar->setObjectName("btnAvisoCancelar");
    btnAvisoCancelar->setCursor(Qt::PointingHandCursor);

    btnAvisoConfirmar = new QPushButton("Sí, desactivar", aviso);
    btnAvisoConfirmar->setObjectName("btnAvisoConfirmar");
    btnAvisoConfirmar->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* layoutAviso = new QHBoxLayout(aviso);
    layoutAviso->setContentsMargins(16, 10, 12, 10);
    layoutAviso->setSpacing(10);
    layoutAviso->addWidget(lblAviso, 1);
    layoutAviso->addWidget(btnAvisoCancelar);
    layoutAviso->addWidget(btnAvisoConfirmar);
    aviso->hide();

    // ----- Ventana completa -----
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(40, 30, 40, 30);
    layoutPrincipal->setSpacing(20);
    layoutPrincipal->addLayout(layoutBarra);
    layoutPrincipal->addLayout(layoutFiltros);
    layoutPrincipal->addWidget(aviso);
    layoutPrincipal->addWidget(tabla, 1);
    setLayout(layoutPrincipal);

    // Hoja de estilos (colores y formas)
    setStyleSheet(R"(
        GestionUsuariosView { background-color: #fff8ee; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTituloGestion { font-size: 26px; font-weight: bold; }
        QPushButton#btnVolver { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 9px 18px; font-weight: bold; }
        QPushButton#btnVolver:hover { background-color: #fffaf3; border: 1px solid #e67e22; }
        QPushButton#btnRegistrar { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 10px 20px; font-weight: bold; }
        QPushButton#btnRegistrar:hover { background-color: #cf6d17; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px 9px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QComboBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 6px 10px; min-height: 22px; min-width: 110px; }
        QComboBox:focus { border: 2px solid #e67e22; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: url(:/ProyectoLaPulpePuraVida/flecha_abajo.png); width: 12px; height: 12px; }
        QComboBox QAbstractItemView { background-color: #ffffff; border: 1px solid #e0c9a6; selection-background-color: #fff3e0; selection-color: #3b2a1a; }
        QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QTableWidget::item { border-bottom: 1px solid #f3e6d0; padding: 6px; }
        QHeaderView { background-color: #fff3e0; border: none; border-top-left-radius: 11px; }
        QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; padding: 0px 10px; }
        QHeaderView::section:first { border-top-left-radius: 11px; }
        QTableWidget QScrollBar:vertical { background: transparent; width: 12px; margin: 44px 0px 12px 0px; border: none; }
        QTableWidget QScrollBar::sub-line:vertical { height: 44px; background-color: #fff3e0; border: none; border-top-right-radius: 11px; subcontrol-position: top; subcontrol-origin: margin; }
        QTableWidget QScrollBar::add-line:vertical { height: 0px; }
        QTableWidget QScrollBar::up-arrow:vertical, QTableWidget QScrollBar::down-arrow:vertical { width: 0px; height: 0px; }
        QTableWidget QScrollBar::handle:vertical { background-color: #e0c9a6; border-radius: 3px; min-height: 30px; margin: 4px 3px 0px 3px; }
        QTableWidget QScrollBar::handle:vertical:hover { background-color: #e67e22; }
        QTableWidget QScrollBar::handle:vertical:disabled { background: transparent; }
        QTableWidget QScrollBar::add-page:vertical, QTableWidget QScrollBar::sub-page:vertical { background: none; }
        QPushButton#btnEditar { background-color: #ffffff; color: #d35400; border: 1px solid #e67e22; border-radius: 6px; padding: 6px 14px; font-weight: bold; }
        QPushButton#btnEditar:hover { background-color: #e67e22; color: #ffffff; }
        QPushButton#btnDesactivar { background-color: #ffffff; color: #c0392b; border: 1px solid #c0392b; border-radius: 6px; padding: 6px 14px; font-weight: bold; }
        QPushButton#btnDesactivar:hover { background-color: #c0392b; color: #ffffff; }
        QPushButton#btnActivar { background-color: #ffffff; color: #1e8449; border: 1px solid #1e8449; border-radius: 6px; padding: 6px 14px; font-weight: bold; }
        QPushButton#btnActivar:hover { background-color: #1e8449; color: #ffffff; }
        QPushButton#btnDesactivar:disabled, QPushButton#btnActivar:disabled { background-color: #f7f2ea; color: #b5a898; border: 1px solid #e0d6c8; }
        QFrame#aviso { background-color: #fdecea; border: 1px solid #e6a59c; border-radius: 8px; }
        QFrame#aviso[tipo="exito"] { background-color: #e8f5e9; border: 1px solid #c8e6c9; }
        QLabel#lblAviso { color: #c0392b; }
        QLabel#lblAviso[tipo="exito"] { color: #1e8449; }
        QPushButton#btnAvisoCancelar { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 6px; padding: 6px 14px; font-weight: bold; }
        QPushButton#btnAvisoCancelar:hover { border: 1px solid #e67e22; }
        QPushButton#btnAvisoConfirmar { background-color: #c0392b; color: #ffffff; border: none; border-radius: 6px; padding: 7px 14px; font-weight: bold; }
        QPushButton#btnAvisoConfirmar:hover { background-color: #a93226; }
    )");

    connect(btnVolver, &QPushButton::clicked, this, &GestionUsuariosView::volverAlMenu);
    connect(btnRegistrar, &QPushButton::clicked, this, &GestionUsuariosView::onBotonRegistrarNuevoClicked);
    connect(btnAvisoCancelar, &QPushButton::clicked, aviso, &QWidget::hide);
    connect(btnAvisoConfirmar, &QPushButton::clicked, this, &GestionUsuariosView::onBotonConfirmarAvisoClicked);

    // Al escribir o cambiar un filtro, la tabla se vuelve a llenar
    connect(txtBuscar, &QLineEdit::textChanged, this, [this](const QString&) { cargarTablaUsuarios(); });
    connect(cmbRol, &QComboBox::currentIndexChanged, this, [this](int) { cargarTablaUsuarios(); });
    connect(cmbEstado, &QComboBox::currentIndexChanged, this, [this](int) { cargarTablaUsuarios(); });

    cargarTablaUsuarios();
}

// ---------------- Métodos privados ----------------

void GestionUsuariosView::cargarTablaUsuarios()
{
    std::vector<std::shared_ptr<Usuario>> usuarios = repo->obtenerTodos();
    std::shared_ptr<Usuario> usuarioEnSesion = Sesion::getInstancia().getUsuarioActual();

    QString busqueda = sinTildes(txtBuscar->text().trimmed());

    tabla->clearSpans();
    tabla->setRowCount(0);

    for (size_t i = 0; i < usuarios.size(); i++)
    {
        std::shared_ptr<Usuario> u = usuarios[i];

        // Filtro por rol y por estado (la posición 0 de cada lista es "Todos")
        if (cmbRol->currentIndex() > 0 && QString::fromStdString(u->getNombreRol()) != cmbRol->currentText())
        {
            continue;
        }
        if (cmbEstado->currentIndex() == 1 && !u->isActivo())
        {
            continue;
        }
        if (cmbEstado->currentIndex() == 2 && u->isActivo())
        {
            continue;
        }

        // Buscador: el texto puede estar en el usuario, el nombre, la cédula o el teléfono
        // (la cédula y el teléfono se pueden escribir con guiones o sin ellos)
        QString cedula = QString::fromStdString(u->getCedula());
        QString telefono = QString::fromStdString(u->getTelefono());
        if (!busqueda.isEmpty()
            && !sinTildes(QString::fromStdString(u->getUsername())).contains(busqueda)
            && !sinTildes(QString::fromStdString(u->getNombreCompleto())).contains(busqueda)
            && !cedula.contains(busqueda) && !QString(cedula).remove('-').contains(busqueda)
            && !telefono.contains(busqueda) && !QString(telefono).remove('-').contains(busqueda))
        {
            continue;
        }

        int fila = tabla->rowCount();
        tabla->insertRow(fila);

        tabla->setItem(fila, 0, crearItem(QString::number(u->getId()), true));
        tabla->setItem(fila, 1, crearItem(QString::fromStdString(u->getUsername()), false));
        // Si el nombre es muy largo y se corta con "...", al pasar el mouse se ve completo
        QTableWidgetItem* itemNombre = crearItem(QString::fromStdString(u->getNombreCompleto()), false);
        itemNombre->setToolTip(itemNombre->text());
        tabla->setItem(fila, 2, itemNombre);
        tabla->setItem(fila, 3, crearItem(QString::fromStdString(u->getCedula()), false));
        tabla->setItem(fila, 4, crearItem(QString::fromStdString(u->getTelefono()), false));
        tabla->setItem(fila, 5, crearItem(QString::fromStdString(u->getNombreRol()), false));

        // Turno: el del cajero; los demás roles devuelven "Sin turno fijo"
        tabla->setItem(fila, 6, crearItem(QString::fromStdString(u->getDescripcionTurno()), false));

        // Estado: verde si está activo, rojo si está inactivo
        QTableWidgetItem* itemEstado = crearItem(u->isActivo() ? "Activo" : "Inactivo", true);
        itemEstado->setForeground(QBrush(QColor(u->isActivo() ? "#1e8449" : "#c0392b")));
        QFont fuente = itemEstado->font();
        fuente.setBold(true);
        itemEstado->setFont(fuente);
        tabla->setItem(fila, 7, itemEstado);

        // Botón para activar o desactivar la cuenta
        QPushButton* btnAccion = new QPushButton(u->isActivo() ? "Desactivar" : "Activar");
        btnAccion->setObjectName(u->isActivo() ? "btnDesactivar" : "btnActivar");
        btnAccion->setCursor(Qt::PointingHandCursor);
        btnAccion->setMinimumSize(110, 34);   // tamaño suficiente para que el texto no se corte

        // La cuenta con la sesión iniciada no se puede desactivar a sí misma
        bool esMiCuenta = (usuarioEnSesion != nullptr && usuarioEnSesion->getId() == u->getId());
        if (esMiCuenta)
        {
            btnAccion->setEnabled(false);
            btnAccion->setToolTip("No puede desactivar su propia cuenta");
        }

        int idUsuario = u->getId();
        connect(btnAccion, &QPushButton::clicked, this, [this, idUsuario]()
            {
                onBotonCambiarEstadoClicked(idUsuario);
            });

        // Botón para modificar el nombre de usuario y el teléfono
        QPushButton* btnEditar = new QPushButton("Editar");
        btnEditar->setObjectName("btnEditar");
        btnEditar->setCursor(Qt::PointingHandCursor);
        btnEditar->setMinimumSize(76, 34);

        connect(btnEditar, &QPushButton::clicked, this, [this, idUsuario]()
            {
                onBotonEditarClicked(idUsuario);
            });

        // Los botones van dentro de un contenedor para que no ocupen toda la celda
        QWidget* contenedor = new QWidget();
        QHBoxLayout* layoutCelda = new QHBoxLayout(contenedor);
        layoutCelda->setContentsMargins(6, 6, 6, 6);
        layoutCelda->setSpacing(6);
        layoutCelda->addStretch();
        layoutCelda->addWidget(btnEditar);
        layoutCelda->addWidget(btnAccion);
        layoutCelda->addStretch();
        tabla->setCellWidget(fila, 8, contenedor);
    }

    // Si ningún usuario cumple los filtros, se avisa en un renglón que ocupa toda la tabla
    if (tabla->rowCount() == 0)
    {
        tabla->insertRow(0);
        tabla->setSpan(0, 0, 1, tabla->columnCount());

        QTableWidgetItem* itemVacio = crearItem("No se encontraron usuarios con esa búsqueda.", true);
        itemVacio->setForeground(QBrush(QColor("#8a7560")));
        tabla->setItem(0, 0, itemVacio);
    }
}

std::shared_ptr<Usuario> GestionUsuariosView::buscarUsuario(int idUsuario)
{
    std::vector<std::shared_ptr<Usuario>> usuarios = repo->obtenerTodos();
    for (size_t i = 0; i < usuarios.size(); i++)
    {
        if (usuarios[i]->getId() == idUsuario)
        {
            return usuarios[i];
        }
    }
    return nullptr;
}

void GestionUsuariosView::cambiarEstado(std::shared_ptr<Usuario> usuario)
{
    usuario->setActivo(!usuario->isActivo());

    if (!repo->actualizar(usuario))
    {
        mostrarAviso("⚠️ No se pudo guardar el cambio. Intente de nuevo.", "error");
        return;
    }

    Bitacora::registrar("Usuarios", usuario->isActivo() ? "Cuenta activada" : "Cuenta desactivada",
        "Usuario: " + usuario->getUsername() + " (" + usuario->getNombreCompleto() + ")");

    QString nombre = QString::fromStdString(usuario->getNombreCompleto());
    mostrarAviso(usuario->isActivo()
        ? "✓ La cuenta de " + nombre + " fue activada. Ya puede iniciar sesión."
        : "✓ La cuenta de " + nombre + " fue desactivada.", "exito");

    // La tabla se actualiza apenas termine este clic (así no se borra el botón mientras se usa)
    QTimer::singleShot(0, this, [this]() { cargarTablaUsuarios(); });
}

void GestionUsuariosView::mostrarAviso(const QString& texto, const QString& tipo)
{
    lblAviso->setText(texto);

    // Los botones solo aparecen cuando se está preguntando algo
    bool esPregunta = (tipo == "confirmar");
    btnAvisoCancelar->setVisible(esPregunta);
    btnAvisoConfirmar->setVisible(esPregunta);

    // Se cambia el color según el tipo y se vuelve a aplicar la hoja de estilos
    aviso->setProperty("tipo", tipo);
    lblAviso->setProperty("tipo", tipo);
    aviso->style()->unpolish(aviso);
    aviso->style()->polish(aviso);
    lblAviso->style()->unpolish(lblAviso);
    lblAviso->style()->polish(lblAviso);

    aviso->show();
}

void GestionUsuariosView::onBotonCambiarEstadoClicked(int idUsuario)
{
    // Doble seguridad: nunca se cambia el estado de la cuenta en sesión
    std::shared_ptr<Usuario> usuarioEnSesion = Sesion::getInstancia().getUsuarioActual();
    if (usuarioEnSesion != nullptr && usuarioEnSesion->getId() == idUsuario)
    {
        return;
    }

    std::shared_ptr<Usuario> usuario = buscarUsuario(idUsuario);
    if (usuario == nullptr)
    {
        return;
    }

    // Para desactivar se pregunta primero en el aviso; activar se hace de una vez
    if (usuario->isActivo())
    {
        idPorDesactivar = idUsuario;
        QString nombre = QString::fromStdString(usuario->getNombreCompleto());
        mostrarAviso("¿Desea desactivar la cuenta de <b>" + nombre + "</b>? "
            "No podrá iniciar sesión hasta que se vuelva a activar.", "confirmar");
        return;
    }

    cambiarEstado(usuario);
}

void GestionUsuariosView::onBotonConfirmarAvisoClicked()
{
    std::shared_ptr<Usuario> usuario = buscarUsuario(idPorDesactivar);
    idPorDesactivar = -1;

    if (usuario == nullptr)
    {
        aviso->hide();
        return;
    }

    cambiarEstado(usuario);
}

void GestionUsuariosView::abrirFormulario(QDialog* dialogo, const QString& mensajeExito)
{
    aviso->hide();

    // Fondo oscuro que cubre toda la ventana (como en la página de referencia)
    QWidget* ventana = window();
    fondoFormulario = new QWidget(ventana);
    fondoFormulario->setObjectName("fondoFormulario");
    fondoFormulario->setAttribute(Qt::WA_StyledBackground, true);
    fondoFormulario->setStyleSheet("QWidget#fondoFormulario { background-color: rgba(42, 37, 35, 165); }");
    fondoFormulario->setGeometry(ventana->rect());

    // El formulario se dibuja dentro de la ventana, centrado sobre el fondo
    dialogo->setParent(fondoFormulario);
    dialogo->setWindowFlags(Qt::Widget);

    QVBoxLayout* layoutFondo = new QVBoxLayout(fondoFormulario);
    layoutFondo->setContentsMargins(0, 0, 0, 0);
    layoutFondo->addWidget(dialogo, 0, Qt::AlignCenter);

    ventana->installEventFilter(this);   // para seguir el tamaño de la ventana
    fondoFormulario->show();
    fondoFormulario->raise();

    // Al cerrar el formulario (Guardar/Confirmar o Cancelar) se quita el fondo
    connect(dialogo, &QDialog::finished, this, [this, ventana, mensajeExito](int resultado)
        {
            ventana->removeEventFilter(this);
            fondoFormulario->deleteLater();
            fondoFormulario = nullptr;

            if (resultado == QDialog::Accepted)
            {
                cargarTablaUsuarios();
                mostrarAviso(mensajeExito, "exito");
            }
        });
}

void GestionUsuariosView::onBotonRegistrarNuevoClicked()
{
    abrirFormulario(new RegistroUsuarioDialog(repo), "✓ El usuario nuevo se registró correctamente.");
}

void GestionUsuariosView::onBotonEditarClicked(int idUsuario)
{
    std::shared_ptr<Usuario> usuario = buscarUsuario(idUsuario);
    if (usuario == nullptr)
    {
        mostrarAviso("⚠️ No se encontró ese usuario. Puede que el archivo haya cambiado.", "error");
        cargarTablaUsuarios();
        return;
    }

    QString nombre = QString::fromStdString(usuario->getNombreCompleto());
    abrirFormulario(new EditarUsuarioDialog(repo, usuario),
        "✓ Los datos de " + nombre + " se actualizaron correctamente.");
}

// ---------------- Eventos ----------------

bool GestionUsuariosView::eventFilter(QObject* objeto, QEvent* evento)
{
    // Si la ventana cambia de tamaño, el fondo oscuro la sigue cubriendo completa
    if (fondoFormulario != nullptr && objeto == window() && evento->type() == QEvent::Resize)
    {
        fondoFormulario->setGeometry(window()->rect());
    }
    return QWidget::eventFilter(objeto, evento);
}