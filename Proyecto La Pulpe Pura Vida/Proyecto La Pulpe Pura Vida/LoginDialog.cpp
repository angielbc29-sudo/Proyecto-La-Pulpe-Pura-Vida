#include "LoginDialog.h"

#include <memory>
#include <string>

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "ServicioAutentication.h"
#include "Sesion.h"
#include "Usuario.h"

//  Constructor 

LoginDialog::LoginDialog(ServicioAutentication* auth, QWidget* parent)
    : QDialog(parent)
{
    this->authService = auth;

    setWindowTitle("La Pulpe Pura Vida - Iniciar sesión");
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumWidth(480);

    // Título
    QLabel* lblTitulo = new QLabel("La Pulpe Pura Vida", this);
    lblTitulo->setObjectName("lblTitulo");
    lblTitulo->setAlignment(Qt::AlignCenter);

    QLabel* lblSistema = new QLabel("Sistema Integral", this);
    lblSistema->setObjectName("lblSistema");
    lblSistema->setAlignment(Qt::AlignCenter);

    // Tarjeta con el formulario 
    QFrame* tarjeta = new QFrame(this);
    tarjeta->setObjectName("tarjetaLogin");

    QLabel* lblBienvenida = new QLabel("¡Hola, Bienvenido a La Pulpe Pura Vida!", tarjeta);
    lblBienvenida->setObjectName("lblBienvenida");
    lblBienvenida->setAlignment(Qt::AlignCenter);
    lblBienvenida->setWordWrap(true);

    // Mensaje de error (empieza oculto)
    lblError = new QLabel(tarjeta);
    lblError->setObjectName("lblError");
    lblError->setWordWrap(true);
    lblError->hide();

    QLabel* lblUsuario = new QLabel("Nombre de Usuario", tarjeta);
    lblUsuario->setObjectName("lblCampo");
    txtUsuario = new QLineEdit(tarjeta);

    QLabel* lblPassword = new QLabel("Contraseña", tarjeta);
    lblPassword->setObjectName("lblCampo");
    txtPassword = new QLineEdit(tarjeta);
    txtPassword->setEchoMode(QLineEdit::Password);   // muestra puntos en vez de letras

    btnIniciarSesion = new QPushButton("Iniciar Sesión", tarjeta);
    btnIniciarSesion->setObjectName("btnIniciarSesion");
    btnIniciarSesion->setCursor(Qt::PointingHandCursor);
    btnIniciarSesion->setDefault(true);

    QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
    layoutTarjeta->setContentsMargins(30, 28, 30, 28);
    layoutTarjeta->setSpacing(8);
    layoutTarjeta->addWidget(lblBienvenida);
    layoutTarjeta->addSpacing(6);
    layoutTarjeta->addWidget(lblError);
    layoutTarjeta->addWidget(lblUsuario);
    layoutTarjeta->addWidget(txtUsuario);
    layoutTarjeta->addSpacing(4);
    layoutTarjeta->addWidget(lblPassword);
    layoutTarjeta->addWidget(txtPassword);
    layoutTarjeta->addSpacing(12);
    layoutTarjeta->addWidget(btnIniciarSesion);

    // Ventana completa 
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(40, 30, 40, 40);
    layoutPrincipal->setSpacing(4);
    layoutPrincipal->addWidget(lblTitulo);
    layoutPrincipal->addWidget(lblSistema);
    layoutPrincipal->addSpacing(18);
    layoutPrincipal->addWidget(tarjeta);
    layoutPrincipal->addStretch();
    setLayout(layoutPrincipal);

    // Hoja de estilos (colores y formas)
    setStyleSheet(R"(
        LoginDialog { background-color: #fff8ee; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTitulo { font-size: 30px; font-weight: bold; color: #d35400; }
        QLabel#lblSistema { font-size: 13px; color: #8a7560; }
        QFrame#tarjetaLogin { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 14px; }
        QLabel#lblBienvenida { font-size: 18px; font-weight: bold; }
        QLabel#lblCampo { font-weight: bold; }
        QLabel#lblError { background-color: #fdecea; color: #c0392b; border: 1px solid #e6a59c; border-radius: 8px; padding: 10px; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 10px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QPushButton#btnIniciarSesion { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 12px; font-weight: bold; font-size: 15px; }
        QPushButton#btnIniciarSesion:hover { background-color: #cf6d17; }
    )");

    // Al hacer clic (o presionar Enter en la contraseña) se intenta iniciar sesión
    connect(btnIniciarSesion, &QPushButton::clicked, this, &LoginDialog::onBotonIniciarSesionClicked);
    connect(txtPassword, &QLineEdit::returnPressed, this, &LoginDialog::onBotonIniciarSesionClicked);
}

// Métodos privados 

void LoginDialog::onBotonIniciarSesionClicked()
{
    std::string user = txtUsuario->text().trimmed().toStdString();
    std::string pass = txtPassword->text().toStdString();

    // Los dos campos son obligatorios
    if (user.empty() || pass.empty())
    {
        lblError->setText("Por favor, complete el nombre de usuario y la contraseña.");
        lblError->show();
        return;
    }

    std::shared_ptr<Usuario> usuario;
    ResultadoLogin resultado = authService->autenticar(user, pass, usuario);

    if (resultado == ResultadoLogin::EXITOSO)
    {
        // Se guarda quién entró y se cierra el login con resultado "aceptado"
        Sesion::getInstancia().iniciar(usuario);
        accept();
    }
    else
    {
        mostrarAlertaError(resultado);
        txtPassword->clear();
    }
}

void LoginDialog::mostrarAlertaError(ResultadoLogin resultado)
{
    switch (resultado)
    {
    case ResultadoLogin::USUARIO_NO_EXISTE:
        lblError->setText("El nombre de usuario ingresado no existe.");
        break;
    case ResultadoLogin::CREDENCIALES_INVALIDAS:
        lblError->setText("La contraseña es incorrecta. Intente de nuevo.");
        break;
    case ResultadoLogin::CUENTA_DESACTIVADA:
        lblError->setText("Acceso denegado: Su cuenta se encuentra inactiva. "
            "Por favor, póngase en contacto con el administrador.");
        break;
    case ResultadoLogin::EXITOSO:
        return;   // no hay nada que mostrar
    }

    lblError->show();
}