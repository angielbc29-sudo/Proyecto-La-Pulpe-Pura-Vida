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
    bool soloReposicion;
    QString textoBusqueda;

    // Métricas superiores
    QLabel* lblTotalUnidades;
    QLabel* lblTotalAgotados;
    QLabel* lblValorTotal;
    QLabel* lblTextoAlerta;

    // Filtros y búsqueda
    QLineEdit* txtBuscador;
    QPushButton* btnFiltroTodos;
    QPushButton* btnFiltroReposicion;
    QPushButton* btnVerReposicionBanner;

    // Pie de página
    QLabel* lblResumenPie;

    // Cuadrícula del estante
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
    void abrirModalDetalleProducto(const std::string& codigo);
    void filtrarPorTexto(const QString& texto);
    void setFiltroTodos();
    void setFiltroReposicion();
};