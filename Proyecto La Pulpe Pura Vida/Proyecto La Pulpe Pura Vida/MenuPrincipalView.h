#pragma once

#include <memory>
#include <QWidget>

class QLabel;
class QPushButton;
class Usuario;

// Vista del menú principal
class MenuPrincipalView : public QWidget
{
    Q_OBJECT

private:
    std::shared_ptr<Usuario> usuarioActivo;

    // Elementos de la interfaz 
    QLabel* lblBienvenida;
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