#include "RegistroUsuarioDialog.h"

#include <memory>
#include <string>
#include <vector>

#include <QAction>
#include <QByteArray>
#include <QComboBox>
#include <QCryptographicHash>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QVBoxLayout>

#include "Administrador.h"
#include "Cajero.h"
#include "IUsuarioRepository.h"
#include "Turno.h"
#include "Usuario.h"

// ---------------- Funciones auxiliares (solo se usan en este archivo) ----------------

namespace
{
    // Dibuja el ícono del ojo (abierto = contraseña oculta, con raya = contraseña visible)
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

        QPainterPath ojo;
        ojo.moveTo(2, 11);
        ojo.quadTo(11, 1, 20, 11);
        ojo.quadTo(11, 21, 2, 11);
        pintor.drawPath(ojo);

        pintor.drawEllipse(QPointF(11, 11), 3.2, 3.2);

        if (tachado)
        {
            pintor.drawLine(QPointF(4, 18), QPointF(18, 4));
        }

        pintor.end();
        return QIcon(pixmap);
    }

    // Deja el campo como contraseña (con puntos) y le agrega el ojo para verla u ocultarla
    void agregarOjo(QLineEdit* campo)
    {
        campo->setEchoMode(QLineEdit::Password);

        QAction* accion = campo->addAction(crearIconoOjo(false), QLineEdit::TrailingPosition);
        accion->setCheckable(true);
        accion->setToolTip("Mostrar contraseña");

        QObject::connect(accion, &QAction::toggled, campo, [campo, accion](bool visible)
            {
                campo->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
                accion->setIcon(crearIconoOjo(visible));
                accion->setToolTip(visible ? "Ocultar contraseña" : "Mostrar contraseña");
            });
    }

    // Junta una etiqueta (con su ayuda en gris) y el campo en un solo bloque
    QWidget* crearCampo(QWidget* padre, const QString& etiqueta, const QString& ayuda, QWidget* campo)
    {
        QWidget* contenedor = new QWidget(padre);

        QLabel* lbl = new QLabel("<b>" + etiqueta + "</b> <span style=\"color:#8a7560;\">" + ayuda + "</span>", contenedor);

        QVBoxLayout* layout = new QVBoxLayout(contenedor);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        layout->addWidget(lbl);
        layout->addWidget(campo);

        return contenedor;
    }

    // Cifra el texto con SHA-256 (mismo método con el que el login compara las contraseñas)
    std::string cifrarSha256(const std::string& texto)
    {
        QByteArray hash = QCryptographicHash::hash(
            QByteArray::fromStdString(texto), QCryptographicHash::Sha256).toHex();
        return hash.toStdString();
    }

    // Texto que se muestra en el aviso cuando no hay ningún error
    const QString AVISO_NORMAL = "Los campos marcados con * son obligatorios.";

    // Cambia el texto del aviso y su color: gris si es normal, rojo si es un error
    void mostrarAviso(QLabel* aviso, const QString& texto, bool esError)
    {
        aviso->setText(texto);
        aviso->setProperty("hayError", esError);

        // Se vuelve a aplicar la hoja de estilos para que tome el color nuevo
        aviso->style()->unpolish(aviso);
        aviso->style()->polish(aviso);
    }
}

// ---------------- Constructor ----------------

RegistroUsuarioDialog::RegistroUsuarioDialog(IUsuarioRepository* repo, QWidget* parent)
    : QDialog(parent)
{
    this->repo = repo;

    setWindowTitle("La Pulpe Pura Vida - Registro de nuevo usuario");
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumWidth(600);

    // ----- Encabezado -----
    QLabel* lblTitulo = new QLabel("Registro de Nuevo Usuario", this);
    lblTitulo->setObjectName("lblTituloRegistro");

    QLabel* lblSubtitulo = new QLabel("Complete los campos obligatorios del personal.", this);
    lblSubtitulo->setObjectName("lblSubtituloRegistro");

    // ----- Campos -----
    txtNombre = new QLineEdit(this);
    txtNombre->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} ]*"), txtNombre));   // solo letras y espacios

    txtCedula = new QLineEdit(this);
    txtCedula->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), txtCedula));       // solo dígitos

    txtTelefono = new QLineEdit(this);
    txtTelefono->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), txtTelefono));   // solo dígitos

    cmbRol = new QComboBox(this);
    cmbRol->addItem("Cajero");
    cmbRol->addItem("Administrador");

    cmbTurno = new QComboBox(this);
    cmbTurno->addItem("Mañana", static_cast<int>(Turno::MANANA));
    cmbTurno->addItem("Tarde", static_cast<int>(Turno::TARDE));
    cmbTurno->addItem("Noche", static_cast<int>(Turno::NOCHE));
    cmbTurno->addItem("Completo", static_cast<int>(Turno::COMPLETO));

    txtUsuario = new QLineEdit(this);
    txtUsuario->setValidator(new QRegularExpressionValidator(QRegularExpression("\\S*"), txtUsuario));       // sin espacios

    txtPassword = new QLineEdit(this);
    agregarOjo(txtPassword);

    txtConfirmar = new QLineEdit(this);
    agregarOjo(txtConfirmar);

    // Aviso arriba del formulario: siempre visible (así la ventana no cambia de tamaño).
    // Normalmente es gris y, si algo está mal, muestra el error en rojo.
    lblError = new QLabel(this);
    lblError->setObjectName("lblError");
    lblError->setWordWrap(true);
    mostrarAviso(lblError, AVISO_NORMAL, false);

    // El turno solo se pide cuando el rol es Cajero
    QWidget* campoTurno = crearCampo(this, "Turno (para cajeros) *", "", cmbTurno);
    connect(cmbRol, &QComboBox::currentIndexChanged, this, [campoTurno](int indice)
        {
            campoTurno->setVisible(indice == 0);
        });

    // ----- Botones -----
    btnCancelar = new QPushButton("Cancelar", this);
    btnCancelar->setObjectName("btnCancelar");
    btnCancelar->setCursor(Qt::PointingHandCursor);

    btnConfirmar = new QPushButton("Confirmar", this);
    btnConfirmar->setObjectName("btnConfirmar");
    btnConfirmar->setCursor(Qt::PointingHandCursor);
    btnConfirmar->setDefault(true);

    QHBoxLayout* layoutBotones = new QHBoxLayout();
    layoutBotones->setSpacing(10);
    layoutBotones->addStretch();
    layoutBotones->addWidget(btnCancelar);
    layoutBotones->addWidget(btnConfirmar);

    // ----- Tarjeta con el formulario (dos columnas) -----
    QFrame* tarjeta = new QFrame(this);
    tarjeta->setObjectName("tarjetaRegistro");

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(8);
    grid->addWidget(crearCampo(this, "Nombre Completo *", "(Solo letras)", txtNombre), 0, 0, 1, 2);
    grid->addWidget(crearCampo(this, "Número de Cédula *", "(Solo dígitos)", txtCedula), 1, 0);
    grid->addWidget(crearCampo(this, "Número de Teléfono *", "(Solo números)", txtTelefono), 1, 1);
    grid->addWidget(crearCampo(this, "Rol *", "", cmbRol), 2, 0);
    grid->addWidget(campoTurno, 2, 1);
    grid->addWidget(crearCampo(this, "Nombre de Usuario deseado *", "(Sin espacios)", txtUsuario), 3, 0, 1, 2);
    grid->addWidget(crearCampo(this, "Contraseña *", "(Mínimo 4 caracteres)", txtPassword), 4, 0);
    grid->addWidget(crearCampo(this, "Confirmar Contraseña *", "", txtConfirmar), 4, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
    layoutTarjeta->setContentsMargins(24, 16, 24, 16);
    layoutTarjeta->setSpacing(10);
    layoutTarjeta->addWidget(lblError);
    layoutTarjeta->addLayout(grid);
    layoutTarjeta->addSpacing(6);
    layoutTarjeta->addLayout(layoutBotones);

    // ----- Ventana completa -----
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(30, 18, 30, 20);
    layoutPrincipal->setSpacing(4);
    layoutPrincipal->addWidget(lblTitulo);
    layoutPrincipal->addWidget(lblSubtitulo);
    layoutPrincipal->addSpacing(12);
    layoutPrincipal->addWidget(tarjeta);
    setLayout(layoutPrincipal);

    // Hoja de estilos (colores y formas)
    setStyleSheet(R"(
        RegistroUsuarioDialog { background-color: #fff8ee; border: 1px solid #f0d9b5; border-radius: 14px; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTituloRegistro { font-size: 24px; font-weight: bold; color: #d35400; }
        QLabel#lblSubtituloRegistro { font-size: 14px; color: #8a7560; }
        QFrame#tarjetaRegistro { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 14px; }
        QLabel#lblError { background-color: #fffaf3; color: #8a7560; border: 1px solid #f0d9b5; border-radius: 8px; padding: 8px 10px; }
        QLabel#lblError[hayError="true"] { background-color: #fdecea; color: #c0392b; border: 1px solid #e6a59c; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px 9px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QComboBox { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 6px 10px; min-height: 22px; }
        QComboBox:focus { border: 2px solid #e67e22; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: url(:/ProyectoLaPulpePuraVida/flecha_abajo.png); width: 12px; height: 12px; }
        QComboBox QAbstractItemView { background-color: #ffffff; border: 1px solid #e0c9a6; selection-background-color: #fff3e0; selection-color: #3b2a1a; }
        QPushButton#btnCancelar { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 10px 22px; font-weight: bold; }
        QPushButton#btnCancelar:hover { background-color: #fffaf3; border: 1px solid #e67e22; }
        QPushButton#btnConfirmar { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 10px 26px; font-weight: bold; }
        QPushButton#btnConfirmar:hover { background-color: #cf6d17; }
    )");

    connect(btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnConfirmar, &QPushButton::clicked, this, &RegistroUsuarioDialog::onBotonConfirmarClicked);
}

// ---------------- Métodos privados ----------------

bool RegistroUsuarioDialog::validarFormulario()
{
    QString nombre = txtNombre->text().trimmed();
    QString cedula = txtCedula->text().trimmed();
    QString telefono = txtTelefono->text().trimmed();
    QString usuario = txtUsuario->text();
    QString password = txtPassword->text();
    QString confirma = txtConfirmar->text();

    QString error;   // se queda vacío si todo está bien

    if (nombre.isEmpty() || cedula.isEmpty() || telefono.isEmpty() ||
        usuario.isEmpty() || password.isEmpty() || confirma.isEmpty())
    {
        error = "Todos los campos obligatorios deben estar completos.";
    }
    else if (!QRegularExpression("^[\\p{L} ]+$").match(nombre).hasMatch())
    {
        error = "El nombre completo solo puede contener letras.";
    }
    else if (!QRegularExpression("^[0-9]+$").match(cedula).hasMatch())
    {
        error = "La cédula solo puede contener dígitos.";
    }
    else if (!QRegularExpression("^[0-9]+$").match(telefono).hasMatch())
    {
        error = "El teléfono solo puede contener números.";
    }
    else if (usuario.contains(' '))
    {
        error = "El nombre de usuario no puede tener espacios.";
    }
    else if (repo->buscarPorUsername(usuario.toStdString()) != nullptr)
    {
        error = "Ese nombre de usuario ya está en uso. Elija otro.";
    }
    else if (password.length() < 4)
    {
        error = "La contraseña debe tener al menos 4 caracteres.";
    }
    else if (password != confirma)
    {
        error = "Las contraseñas no coinciden.";
    }

    if (!error.isEmpty())
    {
        mostrarAviso(lblError, "⚠️ " + error, true);
        return false;
    }

    mostrarAviso(lblError, AVISO_NORMAL, false);
    return true;
}

// ---------------- Eventos ----------------

void RegistroUsuarioDialog::showEvent(QShowEvent* evento)
{
    QDialog::showEvent(evento);
    txtNombre->setFocus();
}

// ---------------- Métodos privados ----------------

void RegistroUsuarioDialog::onBotonConfirmarClicked()
{
    if (!validarFormulario())
    {
        return;
    }

    // El id nuevo es el más alto que ya existe, más 1
    int nuevoId = 1;
    std::vector<std::shared_ptr<Usuario>> todos = repo->obtenerTodos();
    for (size_t i = 0; i < todos.size(); i++)
    {
        if (todos[i]->getId() >= nuevoId)
        {
            nuevoId = todos[i]->getId() + 1;
        }
    }

    std::string nombre = txtNombre->text().trimmed().toStdString();
    std::string cedula = txtCedula->text().trimmed().toStdString();
    std::string telefono = txtTelefono->text().trimmed().toStdString();
    std::string usuario = txtUsuario->text().toStdString();
    std::string hash = cifrarSha256(txtPassword->text().toStdString());

    // Todo usuario nuevo queda activo
    std::shared_ptr<Usuario> nuevo;
    if (cmbRol->currentIndex() == 0)
    {
        Turno turno = static_cast<Turno>(cmbTurno->currentData().toInt());
        nuevo = std::make_shared<Cajero>(nuevoId, usuario, hash, nombre, cedula, telefono, true, turno);
    }
    else
    {
        nuevo = std::make_shared<Administrador>(nuevoId, usuario, hash, nombre, cedula, telefono, true);
    }

    if (repo->guardar(nuevo))
    {
        accept();   // cierra el formulario con resultado "aceptado": se pudo guardar
    }
    else
    {
        mostrarAviso(lblError, "⚠️ No se pudo guardar el usuario. Intente de nuevo.", true);
    }
}