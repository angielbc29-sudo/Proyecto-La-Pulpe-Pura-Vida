#pragma once

#include <memory>
#include <QWidget>

class IUsuarioRepository;
class QComboBox;
class QDialog;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class Usuario;

// Pantalla con la tabla de usuarios (registrar, editar, activar y desactivar)
class GestionUsuariosView : public QWidget
{
    Q_OBJECT

private:
    IUsuarioRepository* repo;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QTableWidget* tabla;
    QPushButton* btnVolver;
    QPushButton* btnRegistrar;

    // Filtros para encontrar un usuario más rápido
    QLineEdit* txtBuscar;
    QComboBox* cmbRol;
    QComboBox* cmbEstado;

    // Aviso dentro de la pantalla (pregunta antes de desactivar o muestra el resultado)
    QFrame* aviso;
    QLabel* lblAviso;
    QPushButton* btnAvisoCancelar;
    QPushButton* btnAvisoConfirmar;
    int idPorDesactivar;    // usuario que se desactiva si se confirma el aviso

    // Fondo oscuro que cubre la ventana mientras el registro o la edición está abierto
    QWidget* fondoFormulario;

    // Vuelve a llenar la tabla con los usuarios guardados que cumplen los filtros
    void cargarTablaUsuarios();

    // Busca un usuario guardado por su id (nullptr si no existe)
    std::shared_ptr<Usuario> buscarUsuario(int idUsuario);

    // Activa o desactiva la cuenta, la guarda y muestra el resultado
    void cambiarEstado(std::shared_ptr<Usuario> usuario);

    // tipo: "confirmar" (rojo con botones), "error" (rojo) o "exito" (verde)
    void mostrarAviso(const QString& texto, const QString& tipo);

    // Muestra el formulario (registro o edición) dentro de la ventana, sobre un fondo oscuro.
    // Si se guarda, recarga la tabla y muestra el mensaje de éxito.
    void abrirFormulario(QDialog* dialogo, const QString& mensajeExito);

private slots:
    void onBotonCambiarEstadoClicked(int idUsuario);
    void onBotonEditarClicked(int idUsuario);
    void onBotonRegistrarNuevoClicked();
    void onBotonConfirmarAvisoClicked();

signals:
    // Avisa al menú principal que se tocó "← Menú Principal"
    void volverAlMenu();

protected:
    // Mantiene el fondo oscuro del formulario del mismo tamaño que la ventana
    bool eventFilter(QObject* objeto, QEvent* evento) override;

public:
    explicit GestionUsuariosView(IUsuarioRepository* repo, QWidget* parent = nullptr);
};
