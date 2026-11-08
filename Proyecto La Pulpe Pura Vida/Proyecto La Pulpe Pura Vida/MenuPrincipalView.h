#pragma once

#include <memory>
#include <QDialog>

#include "UsuarioJsonRepository.h"

class QGridLayout;
class QKeyEvent;
class QLabel;
class QPushButton;
class QStackedWidget;
class Usuario;

// Vista del menú principal (se muestra después de iniciar sesión)
class MenuPrincipalView : public QDialog
{
    Q_OBJECT

private:
    std::shared_ptr<Usuario> usuarioActivo;

    UsuarioJsonRepository repoUsuarios;   // de aquí lee la pantalla de gestión de usuarios

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QLabel* lblIniciales;
    QLabel* lblNombreUsuario;
    QLabel* lblRolUsuario;
    QLabel* lblBienvenida;
    QLabel* lblSubtitulo;
    QGridLayout* gridModulos;
    QPushButton* btnInventario;
    QPushButton* btnVentas;
    QPushButton* btnComprobantes;
    QPushButton* btnUsuarios;
    QPushButton* btnCerrarSesion;

    // Debajo de la cabecera se muestra una sola "página" a la vez:
    // la 0 es el menú con las tarjetas y las demás son los módulos que se abren aquí mismo
    QStackedWidget* paginas;

    void configurarVistaSegunRol();
    void abrirModulo(int idModulo);

private slots:
    void onBotonCerrarSesionClicked();

protected:
    // Evita que la tecla Esc cierre el menú (y con eso todo el programa)
    void keyPressEvent(QKeyEvent* evento) override;

public:
    // Se puede llamar como MenuPrincipalView(); el parent es opcional
    explicit MenuPrincipalView(QWidget* parent = nullptr);
};