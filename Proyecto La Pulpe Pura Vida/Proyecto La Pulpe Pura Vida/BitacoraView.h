#pragma once

#include <vector>
#include <QWidget>
#include "RegistroBitacora.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

// Pantalla del auditor: lista de solo lectura con todo lo registrado en la bitácora
class BitacoraView : public QWidget
{
    Q_OBJECT

private:
    // Todos los renglones de la bitácora, del más nuevo al más viejo
    std::vector<RegistroBitacora> registros;

    // Elementos de la interfaz (se crean por código, sin archivo .ui)
    QPushButton* btnVolver;
    QLineEdit* txtBuscar;
    QComboBox* cmbUsuario;
    QComboBox* cmbModulo;
    QComboBox* cmbFecha;
    QLabel* lblCantidad;
    QTableWidget* tabla;

    // Lee la bitácora y llena las listas de usuarios y módulos de los filtros
    void cargarRegistros();

    // Muestra en la tabla solo los renglones que cumplen los filtros
    void llenarTabla();

signals:
    // Avisa al menú principal que se tocó "← Menú Principal"
    void volverAlMenu();

public:
    explicit BitacoraView(QWidget* parent = nullptr);
};
