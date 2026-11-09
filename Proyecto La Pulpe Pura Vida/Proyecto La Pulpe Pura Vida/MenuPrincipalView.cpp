#include "MenuPrincipalView.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <exception>
#include <memory>

#include "Permiso.h"
#include "Sesion.h"
#include "Usuario.h"

#include "AlmacenJson.h"
#include "Historial.h"
#include "VentanaHistorial.h"
#include "VentanaInventario.h"
#include "GestorInventario.h"
#include "UsuarioJsonRepository.h"
#include "GestionUsuariosView.h"

// ---------------- Función auxiliar (solo se usa en este archivo) ----------------

namespace
{
    // Crea una "tarjeta" clicable con emoji, título y descripción
    QPushButton* crearTarjeta(QWidget* padre, const QString& emoji,
        const QString& titulo, const QString& descripcion)
    {
        QPushButton* tarjeta = new QPushButton(padre);
        tarjeta->setObjectName("tarjeta");
        tarjeta->setCursor(Qt::PointingHandCursor);
        tarjeta->setMinimumHeight(150);
        tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QVBoxLayout* interno = new QVBoxLayout(tarjeta);
        interno->setContentsMargins(20, 18, 20, 18);
        interno->setSpacing(6);

        QLabel* lblEmoji = new QLabel(emoji, tarjeta);
        lblEmoji->setObjectName("tarjetaEmoji");

        QLabel* lblTitulo = new QLabel(titulo, tarjeta);
        lblTitulo->setObjectName("tarjetaTitulo");

        QLabel* lblDescripcion = new QLabel(descripcion, tarjeta);
        lblDescripcion->setObjectName("tarjetaDescripcion");
        lblDescripcion->setWordWrap(true);

        // Los textos no capturan el clic: así toda la tarjeta funciona como un botón
        lblEmoji->setAttribute(Qt::WA_TransparentForMouseEvents);
        lblTitulo->setAttribute(Qt::WA_TransparentForMouseEvents);
        lblDescripcion->setAttribute(Qt::WA_TransparentForMouseEvents);

        interno->addWidget(lblEmoji);
        interno->addWidget(lblTitulo);
        interno->addWidget(lblDescripcion);
        interno->addStretch();

        return tarjeta;
    }
}

// ---------------- Constructor ----------------

MenuPrincipalView::MenuPrincipalView(QWidget* parent)
    : QDialog(parent), repoUsuarios("usuarios.json")
{
    // Se obtiene el usuario que inició sesión
    this->usuarioActivo = Sesion::getInstancia().getUsuarioActual();

    // Los textos de las tarjetas cambian un poco según el rol
    bool esCajero = (usuarioActivo != nullptr && usuarioActivo->getNombreRol() == "Cajero");

    setWindowTitle("La Pulpe Pura Vida - Menú principal");
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);   // botones de minimizar y maximizar

    // ----- Cabecera -----
    QWidget* cabecera = new QWidget(this);
    cabecera->setObjectName("cabecera");

    QLabel* lblTitulo = new QLabel("La Pulpe Pura Vida", cabecera);
    lblTitulo->setObjectName("lblTitulo");
    QLabel* lblSistema = new QLabel("Sistema Integral", cabecera);
    lblSistema->setObjectName("lblSistema");

    lblIniciales = new QLabel(cabecera);
    lblIniciales->setObjectName("lblIniciales");
    lblIniciales->setFixedSize(42, 42);
    lblIniciales->setAlignment(Qt::AlignCenter);

    lblNombreUsuario = new QLabel(cabecera);
    lblNombreUsuario->setObjectName("lblNombreUsuario");
    lblRolUsuario = new QLabel(cabecera);
    lblRolUsuario->setObjectName("lblRolUsuario");

    btnCerrarSesion = new QPushButton("Cerrar Sesión", cabecera);
    btnCerrarSesion->setObjectName("btnCerrarSesion");
    btnCerrarSesion->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* layoutTitulo = new QVBoxLayout();
    layoutTitulo->setSpacing(0);
    layoutTitulo->addWidget(lblTitulo);
    layoutTitulo->addWidget(lblSistema);

    QVBoxLayout* layoutDatosUsuario = new QVBoxLayout();
    layoutDatosUsuario->setSpacing(0);
    layoutDatosUsuario->addWidget(lblNombreUsuario);
    layoutDatosUsuario->addWidget(lblRolUsuario);

    QHBoxLayout* layoutCabecera = new QHBoxLayout(cabecera);
    layoutCabecera->setContentsMargins(30, 14, 30, 14);
    layoutCabecera->setSpacing(12);
    layoutCabecera->addLayout(layoutTitulo);
    layoutCabecera->addStretch();
    layoutCabecera->addWidget(lblIniciales);
    layoutCabecera->addLayout(layoutDatosUsuario);
    layoutCabecera->addSpacing(8);
    layoutCabecera->addWidget(btnCerrarSesion);

    // ----- Cuerpo -----
    QWidget* cuerpo = new QWidget(this);

    lblBienvenida = new QLabel(cuerpo);
    lblBienvenida->setObjectName("lblBienvenida");
    lblSubtitulo = new QLabel(cuerpo);
    lblSubtitulo->setObjectName("lblSubtitulo");

    // Tarjetas de los módulos (según el rol, cambia la descripción)
    btnInventario = crearTarjeta(cuerpo, "📦", "Productos e Inventario",
        "Catálogo «Estante Vivo», existencias, precios y reposición directa.");

    btnVentas = crearTarjeta(cuerpo, "🛒", "Ventas y Cobro",
        esCajero ? "Registro de ventas en tiempo real, cobro de dinero y cálculo de cambio."
        : "Terminal de caja, carrito con validación de stock y cobro en efectivo.");

    btnComprobantes = crearTarjeta(cuerpo, "🧾", "Comprobantes e Historial",
        esCajero ? "Revisión y reimpresión de comprobantes de ventas autorizadas."
        : "Historial permanente de ventas y visor de tickets en TXT / HTML.");

    btnUsuarios = crearTarjeta(cuerpo, "👥", "Usuarios y Permisos",
        "Control de roles, asignación de turnos y activación de personal.");

    gridModulos = new QGridLayout();
    gridModulos->setSpacing(20);
    gridModulos->setColumnStretch(0, 1);
    gridModulos->setColumnStretch(1, 1);

    QVBoxLayout* layoutCuerpo = new QVBoxLayout(cuerpo);
    layoutCuerpo->setContentsMargins(50, 35, 50, 35);
    layoutCuerpo->setSpacing(6);
    layoutCuerpo->addWidget(lblBienvenida);
    layoutCuerpo->addWidget(lblSubtitulo);
    layoutCuerpo->addSpacing(20);
    layoutCuerpo->addLayout(gridModulos);
    layoutCuerpo->addStretch();

    // ----- Páginas (el menú es la primera; los módulos se agregan al abrirlos) -----
    paginas = new QStackedWidget(this);
    paginas->addWidget(cuerpo);

    // ----- Ventana completa -----
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->setSpacing(0);
    layoutPrincipal->addWidget(cabecera);
    layoutPrincipal->addWidget(paginas, 1);
    setLayout(layoutPrincipal);

    setMinimumSize(900, 600);

    // Hoja de estilos (colores y formas)
    setStyleSheet(R"(
        MenuPrincipalView { background-color: #fff8ee; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QWidget#cabecera { background-color: #ffffff; border-bottom: 1px solid #f0d9b5; }
        QLabel { background: transparent; }
        QLabel#lblTitulo { font-size: 26px; font-weight: bold; color: #d35400; }
        QLabel#lblSistema { font-size: 12px; color: #8a7560; }
        QLabel#lblIniciales { background-color: #e67e22; color: white; border-radius: 21px; font-weight: bold; font-size: 15px; }
        QLabel#lblNombreUsuario { font-weight: bold; }
        QLabel#lblRolUsuario { font-size: 12px; color: #8a7560; }
        QLabel#lblBienvenida { font-size: 28px; font-weight: bold; }
        QLabel#lblSubtitulo { font-size: 15px; color: #7a6650; }
        QPushButton#btnCerrarSesion { background-color: #ffffff; color: #c0392b; border: 1px solid #c0392b; border-radius: 8px; padding: 8px 16px; font-weight: bold; }
        QPushButton#btnCerrarSesion:hover { background-color: #c0392b; color: #ffffff; }
        QPushButton#tarjeta { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 14px; text-align: left; }
        QPushButton#tarjeta:hover { background-color: #fffaf3; border: 2px solid #e67e22; }
        QLabel#tarjetaEmoji { font-size: 30px; }
        QLabel#tarjetaTitulo { font-size: 17px; font-weight: bold; }
        QLabel#tarjetaDescripcion { color: #7a6650; }
    )");

    // Cada tarjeta llama a abrirModulo con su número de módulo
    connect(btnInventario, &QPushButton::clicked, this, [this]() { abrirModulo(1); });
    connect(btnVentas, &QPushButton::clicked, this, [this]() { abrirModulo(2); });
    connect(btnComprobantes, &QPushButton::clicked, this, [this]() { abrirModulo(3); });
    connect(btnUsuarios, &QPushButton::clicked, this, [this]() { abrirModulo(4); });
    connect(btnCerrarSesion, &QPushButton::clicked, this, &MenuPrincipalView::onBotonCerrarSesionClicked);

    configurarVistaSegunRol();
}

// ---------------- Métodos privados ----------------

void MenuPrincipalView::configurarVistaSegunRol()
{
    // Si por alguna razón no hay usuario, se oculta todo
    if (usuarioActivo == nullptr)
    {
        lblIniciales->setText("?");
        lblNombreUsuario->setText("");
        lblRolUsuario->setText("");
        lblBienvenida->setText("No hay una sesión iniciada");
        lblSubtitulo->setText("");
        btnInventario->hide();
        btnVentas->hide();
        btnComprobantes->hide();
        btnUsuarios->hide();
        return;
    }

    QString nombre = QString::fromStdString(usuarioActivo->getNombreCompleto());
    QString rol = QString::fromStdString(usuarioActivo->getNombreRol());

    // Iniciales: primera letra de las dos primeras palabras del nombre
    QStringList palabras = nombre.split(' ', Qt::SkipEmptyParts);
    QString iniciales;
    for (int i = 0; i < palabras.size() && i < 2; i++)
    {
        iniciales += palabras[i].left(1).toUpper();
    }

    lblIniciales->setText(iniciales);
    lblNombreUsuario->setText(nombre);
    lblRolUsuario->setText(rol);
    lblBienvenida->setText("¡Bienvenido/a de nuevo, " + nombre + "!");

    if (rol == "Cajero")
    {
        lblSubtitulo->setText("Turno de trabajo asignado: " +
            QString::fromStdString(usuarioActivo->getDescripcionTurno()));
    }
    else
    {
        lblSubtitulo->setText("¿Qué área deseas gestionar hoy?");
    }

    // Cada tarjeta solo se muestra si el usuario tiene el permiso correspondiente
    // (el cajero ve solo Ventas y Comprobantes; el administrador ve los 4 módulos)
    struct Modulo
    {
        QPushButton* boton;
        Permiso permiso;
    };

    Modulo modulos[] = {
        { btnInventario,   Permiso::GESTIONAR_PRODUCTOS },
        { btnVentas,       Permiso::REALIZAR_VENTA },
        { btnComprobantes, Permiso::CONSULTAR_HISTORIAL },
        { btnUsuarios,     Permiso::GESTIONAR_USUARIOS }
    };

    int posicion = 0;
    for (const Modulo& m : modulos)
    {
        if (usuarioActivo->tienePermiso(m.permiso))
        {
            // Se acomodan de dos en dos, sin dejar huecos
            gridModulos->addWidget(m.boton, posicion / 2, posicion % 2);
            m.boton->show();
            posicion++;
        }
        else
        {
            m.boton->hide();
        }
    }
}

void MenuPrincipalView::abrirModulo(int idModulo)
{
    // 1 = Productos/Inventario, 2 = Ventas/Cobro, 3 = Comprobantes/Historial, 4 = Usuarios/Permisos
    // Por ahora solo muestra un aviso. Cuando cada módulo tenga su ventana,
    // aquí se crea y se muestra (por ejemplo: VentasCobro* v = new VentasCobro(); v->show();)
    switch (idModulo)
    {
    case 1:
    {
        // Creamos la bodega, cargamos los datos guardados y abrimos tu ventana
        // Creamos la bodega dinámicamente y abrimos la ventana
        GestorInventario* gestor = new GestorInventario();
        gestor->cargarDesdeArchivo("productos.json");
        VentanaInventario* ventana = new VentanaInventario(gestor);
        ventana->setAttribute(Qt::WA_DeleteOnClose);
        ventana->show();
    }
    break;
    case 2:
        QMessageBox::information(this, "Módulo", "Aquí se abrirá Ventas y cobro");
        break;
    case 3:
    {
        std::shared_ptr<Historial> historial;

        try
        {
            historial = std::make_shared<Historial>(
                std::make_shared<AlmacenJson>("ventas.json"));
        }
        catch (const std::exception&)
        {
            historial.reset();
        }

        VentanaHistorial* vista = new VentanaHistorial(historial, paginas);
        paginas->addWidget(vista);
        paginas->setCurrentWidget(vista);

        connect(vista, &VentanaHistorial::volverAlMenu, this, [this, vista]()
            {
                paginas->setCurrentIndex(0);
                paginas->removeWidget(vista);
                vista->deleteLater();
            });
    }
    break;
    case 4:
    {
        // Pantalla con la tabla de usuarios (se lee del mismo usuarios.json del login).
        // Se abre dentro de esta misma ventana, en lugar de las tarjetas del menú.
        GestionUsuariosView* vista = new GestionUsuariosView(&repoUsuarios, paginas);
        paginas->addWidget(vista);
        paginas->setCurrentWidget(vista);

        // Al tocar "← Menú Principal" se vuelve a las tarjetas y la pantalla se elimina
        connect(vista, &GestionUsuariosView::volverAlMenu, this, [this, vista]()
            {
                paginas->setCurrentIndex(0);
                paginas->removeWidget(vista);
                vista->deleteLater();   // se borra después del clic, no mientras se usa el botón
            });
    }
    break;
    default:
        break;
    }
}

void MenuPrincipalView::onBotonCerrarSesionClicked()
{
    // Se cierran las ventanas de los demás módulos que sigan abiertas (por ejemplo, Inventario)
    const QWidgetList ventanas = QApplication::topLevelWidgets();
    for (QWidget* ventana : ventanas)
    {
        if (ventana != this)
        {
            ventana->close();
        }
    }

    Sesion::getInstancia().cerrar();
    accept();   // cierra el menú con resultado "aceptado": main vuelve a mostrar el login
}

// ---------------- Eventos ----------------

void MenuPrincipalView::keyPressEvent(QKeyEvent* evento)
{
    // En un QDialog, Esc cierra la ventana; en el menú se ignora
    if (evento->key() == Qt::Key_Escape)
    {
        return;
    }

    QDialog::keyPressEvent(evento);
}