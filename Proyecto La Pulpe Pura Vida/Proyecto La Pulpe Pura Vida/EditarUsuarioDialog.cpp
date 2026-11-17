#include "EditarUsuarioDialog.h"

#include <memory>
#include <string>
#include <vector>

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QVBoxLayout>

#include "Bitacora.h"
#include "IUsuarioRepository.h"
#include "Sesion.h"
#include "Usuario.h"

// ---------------- Funciones auxiliares (solo se usan en este archivo) ----------------

namespace
{
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

    // Texto que se muestra en el aviso cuando no hay ningún error
    const QString AVISO_NORMAL = "Solo se pueden cambiar el nombre de usuario y el teléfono.";

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

EditarUsuarioDialog::EditarUsuarioDialog(IUsuarioRepository* repo, std::shared_ptr<Usuario> usuario, QWidget* parent)
    : QDialog(parent)
{
    this->repo = repo;
    this->usuario = usuario;

    setWindowTitle("La Pulpe Pura Vida - Editar usuario");
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumWidth(560);

    // ----- Encabezado -----
    QLabel* lblTitulo = new QLabel("Editar Usuario", this);
    lblTitulo->setObjectName("lblTituloEditar");

    QLabel* lblSubtitulo = new QLabel("Modifique el nombre de usuario o el teléfono del personal.", this);
    lblSubtitulo->setObjectName("lblSubtituloEditar");

    // ----- Datos que no se pueden cambiar (solo se muestran) -----
    QLabel* lblDatosFijos = new QLabel(
        "<b>Nombre:</b> " + QString::fromStdString(usuario->getNombreCompleto()).toHtmlEscaped() +
        " &nbsp;·&nbsp; <b>Rol:</b> " + QString::fromStdString(usuario->getNombreRol()) +
        " &nbsp;·&nbsp; <b>Cédula:</b> " + QString::fromStdString(usuario->getCedula()), this);
    lblDatosFijos->setObjectName("lblDatosFijos");

    // ----- Campos (empiezan con los datos actuales) -----
    txtUsuario = new QLineEdit(this);
    txtUsuario->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9._-]*"), txtUsuario));   // sin espacios, tildes ni ñ
    txtUsuario->setText(QString::fromStdString(usuario->getUsername()));

    txtTelefono = new QLineEdit(this);
    txtTelefono->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,8}"), txtTelefono));   // solo dígitos, máximo 8
    txtTelefono->setText(QString::fromStdString(usuario->getTelefono()).remove('-'));   // se edita sin el guion

    // Aviso arriba del formulario: gris normalmente y rojo si algo está mal
    lblError = new QLabel(this);
    lblError->setObjectName("lblError");
    lblError->setWordWrap(true);
    mostrarAviso(lblError, AVISO_NORMAL, false);

    // ----- Botones -----
    btnCancelar = new QPushButton("Cancelar", this);
    btnCancelar->setObjectName("btnCancelar");
    btnCancelar->setCursor(Qt::PointingHandCursor);

    btnGuardar = new QPushButton("Guardar cambios", this);
    btnGuardar->setObjectName("btnGuardar");
    btnGuardar->setCursor(Qt::PointingHandCursor);
    btnGuardar->setDefault(true);

    QHBoxLayout* layoutBotones = new QHBoxLayout();
    layoutBotones->setSpacing(10);
    layoutBotones->addStretch();
    layoutBotones->addWidget(btnCancelar);
    layoutBotones->addWidget(btnGuardar);

    // ----- Tarjeta con el formulario -----
    QFrame* tarjeta = new QFrame(this);
    tarjeta->setObjectName("tarjetaEditar");

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(8);
    grid->addWidget(crearCampo(this, "Nombre de Usuario *", "(Sin espacios ni tildes)", txtUsuario), 0, 0);
    grid->addWidget(crearCampo(this, "Número de Teléfono *", "(8 dígitos)", txtTelefono), 1, 0);

    QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
    layoutTarjeta->setContentsMargins(24, 16, 24, 16);
    layoutTarjeta->setSpacing(10);
    layoutTarjeta->addWidget(lblError);
    layoutTarjeta->addWidget(lblDatosFijos);
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

    // Hoja de estilos (los mismos colores que el formulario de registro)
    setStyleSheet(R"(
        EditarUsuarioDialog { background-color: #fff8ee; border: 1px solid #f0d9b5; border-radius: 14px; }
        QWidget { font-family: "Segoe UI"; font-size: 14px; color: #3b2a1a; }
        QLabel { background: transparent; }
        QLabel#lblTituloEditar { font-size: 24px; font-weight: bold; color: #d35400; }
        QLabel#lblSubtituloEditar { font-size: 14px; color: #8a7560; }
        QLabel#lblDatosFijos { color: #7a6650; padding: 2px 0px 4px 0px; }
        QFrame#tarjetaEditar { background-color: #ffffff; border: 1px solid #f0d9b5; border-radius: 14px; }
        QLabel#lblError { background-color: #fffaf3; color: #8a7560; border: 1px solid #f0d9b5; border-radius: 8px; padding: 8px 10px; }
        QLabel#lblError[hayError="true"] { background-color: #fdecea; color: #c0392b; border: 1px solid #e6a59c; }
        QLineEdit { background-color: #ffffff; border: 1px solid #e0c9a6; border-radius: 8px; padding: 7px 9px; }
        QLineEdit:focus { border: 2px solid #e67e22; }
        QPushButton#btnCancelar { background-color: #ffffff; color: #7a6650; border: 1px solid #e0c9a6; border-radius: 8px; padding: 10px 22px; font-weight: bold; }
        QPushButton#btnCancelar:hover { background-color: #fffaf3; border: 1px solid #e67e22; }
        QPushButton#btnGuardar { background-color: #e67e22; color: #ffffff; border: none; border-radius: 8px; padding: 10px 26px; font-weight: bold; }
        QPushButton#btnGuardar:hover { background-color: #cf6d17; }
    )");

    connect(btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnGuardar, &QPushButton::clicked, this, &EditarUsuarioDialog::onBotonGuardarClicked);
}

// ---------------- Métodos privados ----------------

bool EditarUsuarioDialog::validarFormulario()
{
    QString nuevoUsuario = txtUsuario->text().trimmed();
    QString telefono = txtTelefono->text().trimmed();

    QString usuarioActual = QString::fromStdString(usuario->getUsername());
    QString telefonoActual = QString::fromStdString(usuario->getTelefono()).remove('-');

    QString error;   // se queda vacío si todo está bien

    if (nuevoUsuario.isEmpty() || telefono.isEmpty())
    {
        error = "El nombre de usuario y el teléfono son obligatorios.";
    }
    else if (!QRegularExpression("^[A-Za-z0-9._-]+$").match(nuevoUsuario).hasMatch())
    {
        error = "El nombre de usuario solo puede tener letras sin tilde, números, punto o guion.";
    }
    else if (!QRegularExpression("^[0-9]{8}$").match(telefono).hasMatch())
    {
        error = "El teléfono debe tener exactamente 8 dígitos.";
    }
    else if (nuevoUsuario == usuarioActual && telefono == telefonoActual)
    {
        error = "No se realizó ningún cambio.";
    }

    // El nombre de usuario y el teléfono no pueden ser los de otra persona
    // (los suyos propios sí se pueden dejar igual)
    if (error.isEmpty())
    {
        std::vector<std::shared_ptr<Usuario>> todos = repo->obtenerTodos();
        for (size_t i = 0; i < todos.size(); i++)
        {
            if (todos[i]->getId() == usuario->getId())
            {
                continue;
            }
            // Se compara sin distinguir mayúsculas, igual que en el login ("carlosm" y "CarlosM" son el mismo)
            if (QString::fromStdString(todos[i]->getUsername()).compare(nuevoUsuario, Qt::CaseInsensitive) == 0)
            {
                error = "Ese nombre de usuario ya está en uso. Elija otro.";
                break;
            }
            if (QString::fromStdString(todos[i]->getTelefono()).remove('-') == telefono)
            {
                error = "Ya existe otro usuario registrado con ese número de teléfono.";
                break;
            }
        }
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

void EditarUsuarioDialog::showEvent(QShowEvent* evento)
{
    QDialog::showEvent(evento);
    txtUsuario->setFocus();
}

// ---------------- Slots ----------------

void EditarUsuarioDialog::onBotonGuardarClicked()
{
    if (!validarFormulario())
    {
        return;
    }

    // Se guardan los datos anteriores por si no se puede escribir el archivo
    std::string usuarioAnterior = usuario->getUsername();
    std::string telefonoAnterior = usuario->getTelefono();

    // El teléfono se guarda con guion, igual que en el registro: 8888-1111
    QString textoTelefono = txtTelefono->text().trimmed();
    std::string nuevoUsuario = txtUsuario->text().trimmed().toStdString();
    std::string telefono = (textoTelefono.left(4) + "-" + textoTelefono.mid(4)).toStdString();

    usuario->setUsername(nuevoUsuario);
    usuario->setTelefono(telefono);

    if (!repo->actualizar(usuario))
    {
        // No se pudo guardar: el usuario queda como estaba
        usuario->setUsername(usuarioAnterior);
        usuario->setTelefono(telefonoAnterior);
        mostrarAviso(lblError, "⚠️ No se pudo guardar el cambio en el archivo. Intente de nuevo.", true);
        return;
    }

    // Si se editó la cuenta con la sesión iniciada, la sesión también toma los datos nuevos
    // (la bitácora y las ventas usan el nombre de usuario de la sesión)
    std::shared_ptr<Usuario> usuarioEnSesion = Sesion::getInstancia().getUsuarioActual();
    if (usuarioEnSesion != nullptr && usuarioEnSesion->getId() == usuario->getId())
    {
        usuarioEnSesion->setUsername(nuevoUsuario);
        usuarioEnSesion->setTelefono(telefono);
    }

    // En la bitácora queda qué cambió (antes -> después)
    std::string detalle = "Usuario: " + nuevoUsuario + " (" + usuario->getNombreCompleto() + ")";
    if (nuevoUsuario != usuarioAnterior)
    {
        detalle += ", nombre de usuario: " + usuarioAnterior + " -> " + nuevoUsuario;
    }
    if (telefono != telefonoAnterior)
    {
        detalle += ", teléfono: " + telefonoAnterior + " -> " + telefono;
    }
    Bitacora::registrar("Usuarios", "Usuario modificado", detalle);

    accept();   // cierra el formulario con resultado "aceptado": se pudo guardar
}
