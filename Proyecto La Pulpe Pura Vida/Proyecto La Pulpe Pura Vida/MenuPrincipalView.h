#pragma once

#include <memory>
#include <QWidget>

class QGridLayout;
class QLabel;
class QPushButton;
class Usuario;

// Vista del menú principal (se muestra después de iniciar sesión)
class MenuPrincipalView : public QWidget
{
    Q_OBJECT

private:
    std::shared_ptr<Usuario> usuarioActivo;

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

    void configurarVistaSegunRol();
    void abrirModulo(int idModulo);

private slots:
    void onBotonCerrarSesionClicked();

public:
    // Se puede llamar como MenuPrincipalView(); el parent es opcional
    explicit MenuPrincipalView(QWidget* parent = nullptr);
};