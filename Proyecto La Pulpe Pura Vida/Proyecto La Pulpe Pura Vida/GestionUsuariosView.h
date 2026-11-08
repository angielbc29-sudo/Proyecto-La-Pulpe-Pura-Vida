#pragma once

#include <memory>
#include <QWidget>

class IUsuarioRepository;
class QFrame;
class QLabel;
class QPushButton;
class QTableWidget;
class Usuario;

// Pantalla con la tabla de usuarios (activar, desactivar y registrar nuevos)
class GestionUsuariosView : public QWidget
{
    Q_OBJECT

private:
    IUsuarioRepository* repo;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QTableWidget* tabla;
    QPushButton* btnVolver;
    QPushButton* btnRegistrar;

    // Aviso dentro de la pantalla (pregunta antes de desactivar o muestra el resultado)
    QFrame* aviso;
    QLabel* lblAviso;
    QPushButton* btnAvisoCancelar;
    QPushButton* btnAvisoConfirmar;
    int idPorDesactivar;    // usuario que se desactiva si se confirma el aviso

    // Fondo oscuro que cubre la ventana mientras el registro está abierto
    QWidget* fondoRegistro;

    // Vuelve a llenar la tabla con los usuarios guardados
    void cargarTablaUsuarios();

    // Busca un usuario guardado por su id (nullptr si no existe)
    std::shared_ptr<Usuario> buscarUsuario(int idUsuario);

    // Activa o desactiva la cuenta, la guarda y muestra el resultado
    void cambiarEstado(std::shared_ptr<Usuario> usuario);

    // tipo: "confirmar" (rojo con botones), "error" (rojo) o "exito" (verde)
    void mostrarAviso(const QString& texto, const QString& tipo);

private slots:
    void onBotonCambiarEstadoClicked(int idUsuario);
    void onBotonRegistrarNuevoClicked();
    void onBotonConfirmarAvisoClicked();

signals:
    // Avisa al menú principal que se tocó "← Menú Principal"
    void volverAlMenu();

protected:
    // Mantiene el fondo oscuro del registro del mismo tamaño que la ventana
    bool eventFilter(QObject* objeto, QEvent* evento) override;

public:
    explicit GestionUsuariosView(IUsuarioRepository* repo, QWidget* parent = nullptr);
};
