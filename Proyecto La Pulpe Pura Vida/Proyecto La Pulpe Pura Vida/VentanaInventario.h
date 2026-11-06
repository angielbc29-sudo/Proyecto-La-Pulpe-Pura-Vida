#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QColor>
#include "GestorInventario.h"

class VentanaInventario : public QWidget {
	Q_OBJECT

private:
	GestorInventario* gestor;//Puntero a la bodega de productos

	//Controles visuales
	QTableWidget* tablaProductos;
	QPushButton* btnNuevoProducto;
	QPushButton* btnCambiarEstado;
	QPushButton* btnGuardar;

	//Configuración visual y carga

	void configurarInterfaz();
	void estilizarVentana();

public: 
	explicit VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent = nullptr);
	~VentanaInventario() = default;

public slots:
	void actualizarTabla();
	void abrirModalNuevoProducto();
	void alternarEstadoSeleccionado();
	void guardarCambiosEnDisco();
};