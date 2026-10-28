#include "MenuPrincipalView.h"

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "Permiso.h"
#include "Sesion.h"
#include "Usuario.h"

MenuPrincipalView::MenuPrincipalView(QWidget* parent)
    : QWidget(parent)
{
    // Se obtiene el usuario que inició sesión
    this->usuarioActivo = Sesion::getInstancia().getUsuarioActual();

    setWindowTitle("Menú principal - La Pulpe Pura Vida");

    // Se crean los elementos de la interfaz
    lblBienvenida = new QLabel(this);
    btnInventario = new QPushButton("Productos e inventario", this);
    btnVentas = new QPushButton("Ventas y cobro", this);
    btnComprobantes = new QPushButton("Comprobantes e historial", this);
    btnUsuarios = new QPushButton("Usuarios y permisos", this);
    btnCerrarSesion = new QPushButton("Cerrar sesión", this);

    // Se acomodan uno debajo del otro
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(lblBienvenida);
    layout->addWidget(btnInventario);
    layout->addWidget(btnVentas);
    layout->addWidget(btnComprobantes);
    layout->addWidget(btnUsuarios);
    layout->addWidget(btnCerrarSesion);
    setLayout(layout);

    // Cada botón de módulo llama a abrirModulo con su número
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
        lblBienvenida->setText("No hay una sesión iniciada");
        btnInventario->setVisible(false);
        btnVentas->setVisible(false);
        btnComprobantes->setVisible(false);
        btnUsuarios->setVisible(false);
        return;
    }

    lblBienvenida->setText(
        "Bienvenido/a, " + QString::fromStdString(usuarioActivo->getNombreCompleto()) +
        "\nRol: " + QString::fromStdString(usuarioActivo->getNombreRol()) +
        "\n" + QString::fromStdString(usuarioActivo->getDescripcionTurno()));

    // Cada botón solo se muestra si el usuario tiene el permiso correspondiente
    // (el cajero ve solo Ventas y Comprobantes; el administrador ve los 4 módulos)
    btnInventario->setVisible(usuarioActivo->tienePermiso(Permiso::GESTIONAR_PRODUCTOS));
    btnVentas->setVisible(usuarioActivo->tienePermiso(Permiso::REALIZAR_VENTA));
    btnComprobantes->setVisible(usuarioActivo->tienePermiso(Permiso::CONSULTAR_HISTORIAL));
    btnUsuarios->setVisible(usuarioActivo->tienePermiso(Permiso::GESTIONAR_USUARIOS));
}

void MenuPrincipalView::abrirModulo(int idModulo)
{
    // 1 = Productos/Inventario, 2 = Ventas/Cobro, 3 = Comprobantes/Historial, 4 = Usuarios/Permisos
    // Por ahora solo muestra un aviso. Cuando cada módulo tenga su ventana,
    // aquí se crea y se muestra (por ejemplo: VentasCobro* v = new VentasCobro(); v->show();)
    switch (idModulo)
    {
    case 1:
        QMessageBox::information(this, "Módulo", "Aquí se abrirá Productos e inventario");
        break;
    case 2:
        QMessageBox::information(this, "Módulo", "Aquí se abrirá Ventas y cobro");
        break;
    case 3:
        QMessageBox::information(this, "Módulo", "Aquí se abrirá Comprobantes e historial");
        break;
    case 4:
        QMessageBox::information(this, "Módulo", "Aquí se abrirá Usuarios y permisos");
        break;
    default:
        break;
    }
}

void MenuPrincipalView::onBotonCerrarSesionClicked()
{
    Sesion::getInstancia().cerrar();
    close();
}