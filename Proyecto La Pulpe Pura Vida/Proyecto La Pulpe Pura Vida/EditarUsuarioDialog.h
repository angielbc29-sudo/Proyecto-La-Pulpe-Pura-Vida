#pragma once

#include <memory>
#include <QDialog>

class IUsuarioRepository;
class QLabel;
class QLineEdit;
class QPushButton;
class Usuario;

// Formulario para modificar un usuario que ya existe (solo nombre de usuario y teléfono)
class EditarUsuarioDialog : public QDialog
{
    Q_OBJECT

private:
    IUsuarioRepository* repo;
    std::shared_ptr<Usuario> usuario;   // usuario que se está editando

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QLineEdit* txtUsuario;
    QLineEdit* txtTelefono;
    QLabel* lblError;
    QPushButton* btnCancelar;
    QPushButton* btnGuardar;

    // Revisa los campos; si algo está mal, muestra el error y devuelve false
    bool validarFormulario();

private slots:
    void onBotonGuardarClicked();

protected:
    // Al aparecer, el cursor queda listo en "Nombre de Usuario"
    void showEvent(QShowEvent* evento) override;

public:
    EditarUsuarioDialog(IUsuarioRepository* repo, std::shared_ptr<Usuario> usuario, QWidget* parent = nullptr);
};
