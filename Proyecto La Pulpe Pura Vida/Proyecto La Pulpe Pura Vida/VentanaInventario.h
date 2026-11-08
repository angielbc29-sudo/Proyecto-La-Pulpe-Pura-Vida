#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <vector>
#include "GestorInventario.h"

class VentanaInventario : public QWidget {
    Q_OBJECT

private:
    GestorInventario* gestor;

    // Métricas superiores
    QLabel* lblTotalUnidades;
    QLabel* lblTotalAgotados;
    QLabel* lblValorTotal;
    QLabel* lblTextoAlerta;

    // Contenedor de la cuadrícula de tarjetas
    QWidget* contenedorCards;
    QGridLayout* gridProductos;

    void configurarInterfaz();
    void estilizarVentana();
    QWidget* crearCardProducto(const Producto& prod);

public:
    explicit VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent = nullptr);
    ~VentanaInventario() = default;

public slots:
    void actualizarVista();
    void abrirModalNuevoProducto();
    void guardarCambiosEnDisco();
};