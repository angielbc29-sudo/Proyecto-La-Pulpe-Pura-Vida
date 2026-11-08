#include "GestionUsuariosView.h"

#include <memory>
#include <vector>

#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include "IUsuarioRepository.h"
#include "RegistroUsuarioDialog.h"
#include "Sesion.h"
#include "Usuario.h"

// ---------------- Función auxiliar (solo se usa en este archivo) ----------------

namespace
{
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
    this->fondoRegistro = nullptr;

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

    // ----- Tabla -----
    tabla = new QTableWidget(this);
    tabla->setColumnCount(8);
    tabla->setHorizontalHeaderLabels({ "N.°", "Usuario", "Nombre Completo", "Cédula",
                                       "Teléfono", "Rol", "Estado", "Acción" });
    tabla->verticalHeader()->setVisible(false);
    tabla->verticalHeader()->setDefaultSectionSize(56);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setSelectionMode(QAbstractItemView::NoSelection);
    tabla->setFocusPolicy(Qt::NoFocus);
    tabla->setShowGrid(false);

    QHeaderView* cabecera = tabla->horizontalHeader();
    cabecera->setSectionResizeMode(QHeaderView::ResizeToContents);
    cabecera->setSectionResizeMode(2, QHeaderView::Stretch);   // el nombre ocupa el espacio que sobre
    cabecera->setSectionResizeMode(7, QHeaderView::Fixed);     // la columna del botón tiene un ancho fijo
    tabla->setColumnWidth(7, 150);
    cabecera->setHighlightSections(false);

    // Los títulos de las columnas de texto van a la izquierda, igual que su contenido
    for (int columna = 1; columna <= 5; columna++)
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
        QTableWidget { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 12px; }
        QTableWidget::item { border-bottom: 1px solid #f3e6d0; padding: 6px; }
        QHeaderView::section { background-color: #fff3e0; color: #7a6650; font-weight: bold; border: none; border-bottom: 1px solid #f0d9b5; padding: 10px; }
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

    cargarTablaUsuarios();
}

// ---------------- Métodos privados ----------------

void GestionUsuariosView::cargarTablaUsuarios()
{
    std::vector<std::shared_ptr<Usuario>> usuarios = repo->obtenerTodos();
    std::shared_ptr<Usuario> usuarioEnSesion = Sesion::getInstancia().getUsuarioActual();

    tabla->setRowCount(0);
    tabla->setRowCount(static_cast<int>(usuarios.size()));

    for (int fila = 0; fila < static_cast<int>(usuarios.size()); fila++)
    {
        std::shared_ptr<Usuario> u = usuarios[fila];

        tabla->setItem(fila, 0, crearItem(QString::number(u->getId()), true));
        tabla->setItem(fila, 1, crearItem(QString::fromStdString(u->getUsername()), false));
        tabla->setItem(fila, 2, crearItem(QString::fromStdString(u->getNombreCompleto()), false));
        tabla->setItem(fila, 3, crearItem(QString::fromStdString(u->getCedula()), false));
        tabla->setItem(fila, 4, crearItem(QString::fromStdString(u->getTelefono()), false));
        tabla->setItem(fila, 5, crearItem(QString::fromStdString(u->getNombreRol()), false));

        // Estado: verde si está activo, rojo si está inactivo
        QTableWidgetItem* itemEstado = crearItem(u->isActivo() ? "Activo" : "Inactivo", true);
        itemEstado->setForeground(QBrush(QColor(u->isActivo() ? "#1e8449" : "#c0392b")));
        QFont fuente = itemEstado->font();
        fuente.setBold(true);
        itemEstado->setFont(fuente);
        tabla->setItem(fila, 6, itemEstado);

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
            btnAccion->setToolTip("No puedes desactivar tu propia cuenta");
        }

        int idUsuario = u->getId();
        connect(btnAccion, &QPushButton::clicked, this, [this, idUsuario]()
            {
                onBotonCambiarEstadoClicked(idUsuario);
            });

        // El botón va dentro de un contenedor para que no ocupe toda la celda
        QWidget* contenedor = new QWidget();
        QHBoxLayout* layoutCelda = new QHBoxLayout(contenedor);
        layoutCelda->setContentsMargins(10, 6, 10, 6);
        layoutCelda->addWidget(btnAccion, 0, Qt::AlignCenter);
        tabla->setCellWidget(fila, 7, contenedor);
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

void GestionUsuariosView::onBotonRegistrarNuevoClicked()
{
    aviso->hide();

    // Fondo oscuro que cubre toda la ventana (como en la página de referencia)
    QWidget* ventana = window();
    fondoRegistro = new QWidget(ventana);
    fondoRegistro->setObjectName("fondoRegistro");
    fondoRegistro->setAttribute(Qt::WA_StyledBackground, true);
    fondoRegistro->setStyleSheet("QWidget#fondoRegistro { background-color: rgba(42, 37, 35, 165); }");
    fondoRegistro->setGeometry(ventana->rect());

    // El formulario se dibuja dentro de la ventana, centrado sobre el fondo
    RegistroUsuarioDialog* dialogo = new RegistroUsuarioDialog(repo, fondoRegistro);
    dialogo->setWindowFlags(Qt::Widget);

    QVBoxLayout* layoutFondo = new QVBoxLayout(fondoRegistro);
    layoutFondo->setContentsMargins(0, 0, 0, 0);
    layoutFondo->addWidget(dialogo, 0, Qt::AlignCenter);

    ventana->installEventFilter(this);   // para seguir el tamaño de la ventana
    fondoRegistro->show();
    fondoRegistro->raise();

    // Al cerrar el formulario (Confirmar o Cancelar) se quita el fondo
    connect(dialogo, &QDialog::finished, this, [this, ventana](int resultado)
        {
            ventana->removeEventFilter(this);
            fondoRegistro->deleteLater();
            fondoRegistro = nullptr;

            if (resultado == QDialog::Accepted)
            {
                cargarTablaUsuarios();
                mostrarAviso("✓ El usuario nuevo se registró correctamente.", "exito");
            }
        });
}

// ---------------- Eventos ----------------

bool GestionUsuariosView::eventFilter(QObject* objeto, QEvent* evento)
{
    // Si la ventana cambia de tamaño, el fondo oscuro la sigue cubriendo completa
    if (fondoRegistro != nullptr && objeto == window() && evento->type() == QEvent::Resize)
    {
        fondoRegistro->setGeometry(window()->rect());
    }
    return QWidget::eventFilter(objeto, evento);
}