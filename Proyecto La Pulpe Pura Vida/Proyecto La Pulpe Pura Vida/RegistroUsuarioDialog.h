#pragma once

#include <QDialog>

class IUsuarioRepository;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

// Formulario para registrar un usuario nuevo
class RegistroUsuarioDialog : public QDialog
{
    Q_OBJECT

private:
    IUsuarioRepository* repo;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QLineEdit* txtNombre;
    QLineEdit* txtCedula;
    QLineEdit* txtTelefono;
    QComboBox* cmbRol;
    QComboBox* cmbTurno;
    QLineEdit* txtUsuario;
    QLineEdit* txtPassword;
    QLineEdit* txtConfirmar;
    QLabel* lblError;
    QPushButton* btnCancelar;
    QPushButton* btnConfirmar;

    // Revisa todos los campos; si algo está mal, muestra el error y devuelve false
    bool validarFormulario();

private slots:
    void onBotonConfirmarClicked();

public:
    explicit RegistroUsuarioDialog(IUsuarioRepository* repo, QWidget* parent = nullptr);
};