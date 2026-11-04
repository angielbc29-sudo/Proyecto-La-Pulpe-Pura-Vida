#pragma once

#include <QDialog>
#include "ResultadoLogin.h"

class QLabel;
class QLineEdit;
class QPushButton;
class ServicioAutentication;

// Pantalla de inicio de sesión
class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    ServicioAutentication* authService;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QLineEdit* txtUsuario;
    QLineEdit* txtPassword;
    QPushButton* btnIniciarSesion;
    QLabel* lblError;

    void mostrarAlertaError(ResultadoLogin resultado);

private slots:
    void onBotonIniciarSesionClicked();

public:
    explicit LoginDialog(ServicioAutentication* auth, QWidget* parent = nullptr);
};