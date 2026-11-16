#pragma once

#include <QDialog>
#include "ResultadoLogin.h"

class QKeyEvent;
class QLabel;
class QLineEdit;
class QPushButton;
class ServicioAutenticacion;

// Pantalla de inicio de sesión
class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    ServicioAutenticacion* authService;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QLineEdit* txtUsuario;
    QLineEdit* txtPassword;
    QPushButton* btnIniciarSesion;
    QLabel* lblError;

    void mostrarAlertaError(ResultadoLogin resultado);

private slots:
    void onBotonIniciarSesionClicked();

protected:
    // Evita que la tecla Esc cierre el login (y con eso todo el programa)
    void keyPressEvent(QKeyEvent* evento) override;

public:
    explicit LoginDialog(ServicioAutenticacion* auth, QWidget* parent = nullptr);
};