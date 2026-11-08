#include "LoginDialog.h"

#include <memory>
#include <string>

#include <QAction>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "ServicioAutentication.h"
#include "Sesion.h"
#include "Usuario.h"

// ---------------- Función auxiliar (solo se usa en este archivo) ----------------

namespace
{
    // Dibuja el ícono del ojo (como el de las páginas web).
    // tachado = false -> ojo abierto (la contraseña está oculta, tócalo para verla)
    // tachado = true  -> ojo con una raya (la contraseña se está viendo, tócalo para ocultarla)
    QIcon crearIconoOjo(bool tachado)
    {
        const int tam = 22;
        const qreal escala = 2.0;   // se dibuja al doble de tamaño para que se vea nítido

        QPixmap pixmap(int(tam * escala), int(tam * escala));
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(escala);

        QPainter pintor(&pixmap);
        pintor.setRenderHint(QPainter::Antialiasing, true);

        QPen lapiz(QColor("#7a6650"));
        lapiz.setWidthF(1.8);
        lapiz.setCapStyle(Qt::RoundCap);
        lapiz.setJoinStyle(Qt::RoundJoin);
        pintor.setPen(lapiz);
        pintor.setBrush(Qt::NoBrush);

        // Contorno del ojo (forma de almendra)
        QPainterPath ojo;
        ojo.moveTo(2, 11);
        ojo.quadTo(11, 1, 20, 11);
        ojo.quadTo(11, 21, 2, 11);
        pintor.drawPath(ojo);

        // Pupila
        pintor.drawEllipse(QPointF(11, 11), 3.2, 3.2);

        // Raya diagonal cuando la contraseña está visible
        if (tachado)
        {
            pintor.drawLine(QPointF(4, 18), QPointF(18, 4));
        }

        pintor.end();
        return QIcon(pixmap);
    }
}

// ---------------- Constructor ----------------

LoginDialog::LoginDialog(ServicioAutenticacion* auth, QWidget* parent)
    : QDialog(parent)
{
    this->authService = auth;

    setWindowTitle("La Pulpe Pura Vida - Iniciar sesión");
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumWidth(480);

    // ----- Título -----
    QLabel* lblTitulo = new QLabel("La Pulpe Pura Vida", this);
    lblTitulo->setObjectName("lblTitulo");
    lblTitulo->setAlignment(Qt::AlignCenter);

    QLabel* lblSistema = new QLabel("Sistema Integral", this);
    lblSistema->setObjectName("lblSistema");
    lblSistema->setAlignment(Qt::AlignCenter);

    // ----- Tarjeta con el formulario -----
    QFrame* tarjeta = new QFrame(this);
    tarjeta->setObjectName("tarjetaLogin");

    QLabel* lblEncabezado = new QLabel("Iniciar sesión", tarjeta);
    lblEncabezado->setObjectName("lblEncabezado");
    lblEncabezado->setAlignment(Qt::AlignCenter);
    lblEncabezado->setWordWrap(true);

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

    // Ícono del ojo dentro del campo, a la derecha (como en las páginas web)
    QAction* accionVerPassword = txtPassword->addAction(crearIconoOjo(false), QLineEdit::TrailingPosition);
    accionVerPassword->setCheckable(true);
    accionVerPassword->setToolTip("Mostrar contraseña");

    btnIniciarSesion = new QPushButton("Iniciar Sesión", tarjeta);
    btnIniciarSesion->setObjectName("btnIniciarSesion");
    btnIniciarSesion->setCursor(Qt::PointingHandCursor);
    btnIniciarSesion->setDefault(true);

    QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
    layoutTarjeta->setContentsMargins(30, 28, 30, 28);
    layoutTarjeta->setSpacing(8);
    layoutTarjeta->addWidget(lblEncabezado);
    layoutTarjeta->addSpacing(6);
    layoutTarjeta->addWidget(lblError);
    layoutTarjeta->addWidget(lblUsuario);
    layoutTarjeta->addWidget(txtUsuario);
    layoutTarjeta->addSpacing(4);
    layoutTarjeta->addWidget(lblPassword);
    layoutTarjeta->addWidget(txtPassword);
    layoutTarjeta->addSpacing(12);
    layoutTarjeta->addWidget(btnIniciarSesion);

    // ----- Ventana completa -----
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
        QLabel#lblEncabezado { font-size: 18px; font-weight: bold; }
        QLabel#lblCampo { font-weight: bold; }
        QLabel#lblError { background-color: #fdecea; color: #c0392b; border: 1px solid #e6a59c; border-radius: 8px; padding: 10px; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 10px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QPushButton#btnIniciarSesion { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 12px; font-weight: bold; font-size: 15px; }
        QPushButton#btnIniciarSesion:hover { background-color: #cf6d17; }
    )");

    // Al tocar el ojo se alterna entre ver y ocultar la contraseña
    connect(accionVerPassword, &QAction::toggled, this, [this, accionVerPassword](bool visible)
        {
            txtPassword->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
            accionVerPassword->setIcon(crearIconoOjo(visible));
            accionVerPassword->setToolTip(visible ? "Ocultar contraseña" : "Mostrar contraseña");
        });

    // Al hacer clic (o presionar Enter en la contraseña) se intenta iniciar sesión
    connect(btnIniciarSesion, &QPushButton::clicked, this, &LoginDialog::onBotonIniciarSesionClicked);
    connect(txtPassword, &QLineEdit::returnPressed, this, &LoginDialog::onBotonIniciarSesionClicked);
}

// ---------------- Métodos privados ----------------

void LoginDialog::onBotonIniciarSesionClicked()
{
    std::string user = txtUsuario->text().trimmed().toStdString();
    std::string pass = txtPassword->text().toStdString();

    // Los dos campos son obligatorios
    if (user.empty() || pass.empty())
    {
        lblError->setText("⚠️ Por favor, complete el nombre de usuario y la contraseña.");
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
        lblError->setText("⚠️ El nombre de usuario ingresado no existe.");
        break;
    case ResultadoLogin::CREDENCIALES_INVALIDAS:
        lblError->setText("⚠️ La contraseña es incorrecta. Intente de nuevo.");
        break;
    case ResultadoLogin::CUENTA_DESACTIVADA:
        lblError->setText("⚠️ Acceso denegado: Su cuenta se encuentra inactiva. "
            "Por favor, póngase en contacto con el administrador.");
        break;
    case ResultadoLogin::EXITOSO:
        return;   // no hay nada que mostrar
    }

    lblError->show();
}