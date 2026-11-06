#include "VentanaInventario.h"
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QMessageBox>

// =====================================================
// CONSTRUCTOR: Inicializa la ventana y conecta la bodega
// =====================================================
VentanaInventario::VentanaInventario(GestorInventario* gestorCompartido, QWidget* parent)
    : QWidget(parent), gestor(gestorCompartido)
{
    configurarInterfaz();
    estilizarVentana();
    actualizarTabla();
}

// =====================================================
// CONFIGURACIÓN DE CONTROLES Y EVENTOS
// =====================================================
void VentanaInventario::configurarInterfaz() {
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    // 1. Título superior
    QLabel* lblTitulo = new QLabel("Gestión de Inventario — La Pulpe", this);
    lblTitulo->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e3a8a; margin-bottom: 10px;");
    layoutPrincipal->addWidget(lblTitulo);

    // 2. Tabla de productos
    tablaProductos = new QTableWidget(this);
    tablaProductos->setColumnCount(6);
    QStringList encabezados = { "Código", "Nombre", "Categoría", "Precio (₡)", "Stock", "Estado" };
    tablaProductos->setHorizontalHeaderLabels(encabezados);
    tablaProductos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProductos->setEditTriggers(QAbstractItemView::NoEditTriggers); // Solo lectura en la tabla
    layoutPrincipal->addWidget(tablaProductos);

    // 3. Barra inferior de botones
    QHBoxLayout* layoutBotones = new QHBoxLayout();

    btnNuevoProducto = new QPushButton("Nuevo Producto", this);
    btnCambiarEstado = new QPushButton("Activar / Desactivar", this);
    btnGuardar = new QPushButton("Guardar Cambios", this);

    layoutBotones->addWidget(btnNuevoProducto);
    layoutBotones->addWidget(btnCambiarEstado);
    layoutBotones->addStretch(); // Empuja el botón de guardar a la derecha
    layoutBotones->addWidget(btnGuardar);

    layoutPrincipal->addLayout(layoutBotones);

    // 4. Conexiones (Señales -> Slots)
    connect(btnNuevoProducto, &QPushButton::clicked, this, &VentanaInventario::abrirModalNuevoProducto);
    connect(btnCambiarEstado, &QPushButton::clicked, this, &VentanaInventario::alternarEstadoSeleccionado);
    connect(btnGuardar, &QPushButton::clicked, this, &VentanaInventario::guardarCambiosEnDisco);
}

// =====================================================
// ESTILOS VISUALES (Colores limpios y profesionales)
// =====================================================
void VentanaInventario::estilizarVentana() {
    btnNuevoProducto->setStyleSheet("background-color: #0d9488; color: white; font-weight: bold; padding: 8px 14px; border-radius: 4px;");
    btnCambiarEstado->setStyleSheet("background-color: #f59e0b; color: white; font-weight: bold; padding: 8px 14px; border-radius: 4px;");
    btnGuardar->setStyleSheet("background-color: #1e3a8a; color: white; font-weight: bold; padding: 8px 14px; border-radius: 4px;");
}

// =====================================================
// LLENAR LA TABLA CON LOS PRODUCTOS Y ALERTAS DE STOCK
// =====================================================
void VentanaInventario::actualizarTabla() {
    if (gestor == nullptr) return;

    tablaProductos->setRowCount(0); // Limpiar filas

    auto productos = gestor->getProductosActivos();
    for (const auto& prod : productos) {
        int fila = tablaProductos->rowCount();
        tablaProductos->insertRow(fila);

        QTableWidgetItem* itemCodigo = new QTableWidgetItem(QString::fromStdString(prod.getCodigo()));
        QTableWidgetItem* itemNombre = new QTableWidgetItem(QString::fromStdString(prod.getNombre()));
        QTableWidgetItem* itemCat = new QTableWidgetItem(QString::fromStdString(prod.getCategoria()));
        QTableWidgetItem* itemPrecio = new QTableWidgetItem(QString::number(prod.getPrecio(), 'f', 2));
        QTableWidgetItem* itemStock = new QTableWidgetItem(QString::number(prod.getStock()));
        QTableWidgetItem* itemEstado = new QTableWidgetItem(prod.getActivo() ? "Activo" : "Inactivo");

        // Regla de Negocio: Alerta visual de stock crítico (<= 5 unidades)
        if (prod.getStock() <= 5) {
            QColor colorAlerta = (prod.getStock() == 0) ? QColor("#fecaca") : QColor("#fef08a"); // Rojo si es 0, amarillo si <= 5
            itemStock->setBackground(colorAlerta);
            itemStock->setToolTip("Stock bajo en bodega");
        }

        tablaProductos->setItem(fila, 0, itemCodigo);
        tablaProductos->setItem(fila, 1, itemNombre);
        tablaProductos->setItem(fila, 2, itemCat);
        tablaProductos->setItem(fila, 3, itemPrecio);
        tablaProductos->setItem(fila, 4, itemStock);
        tablaProductos->setItem(fila, 5, itemEstado);
    }
}

// =====================================================
// FORMULARIO MODAL: AGREGAR NUEVO PRODUCTO
// =====================================================
void VentanaInventario::abrirModalNuevoProducto() {
    QDialog modal(this);
    modal.setWindowTitle("Registrar Nuevo Producto");
    modal.setFixedWidth(350);

    QFormLayout form(&modal);

    QLineEdit txtCodigo(&modal);
    QLineEdit txtNombre(&modal);
    QLineEdit txtCategoria(&modal);

    QDoubleSpinBox spinPrecio(&modal);
    spinPrecio.setRange(1.0, 1000000.0);
    spinPrecio.setPrefix("₡ ");

    QSpinBox spinStock(&modal);
    spinStock.setRange(0, 10000);

    form.addRow("Código:", &txtCodigo);
    form.addRow("Nombre:", &txtNombre);
    form.addRow("Categoría:", &txtCategoria);
    form.addRow("Precio:", &spinPrecio);
    form.addRow("Stock inicial:", &spinStock);

    QPushButton btnAceptar("Guardar Producto", &modal);
    btnAceptar.setStyleSheet("background-color: #0d9488; color: white; font-weight: bold; padding: 6px;");
    form.addRow(&btnAceptar);

    connect(&btnAceptar, &QPushButton::clicked, [&]() {
        std::string cod = txtCodigo.text().trimmed().toStdString();
        std::string nom = txtNombre.text().trimmed().toStdString();
        std::string cat = txtCategoria.text().trimmed().toStdString();
        double precio = spinPrecio.value();
        int stock = spinStock.value();

        // Validaciones de entrada
        if (cod.empty() || nom.empty()) {
            QMessageBox::warning(&modal, "Dato Faltante", "El código y el nombre no pueden estar vacíos.");
            return;
        }

        if (!gestor->validarCodigoUnico(cod)) {
            QMessageBox::warning(&modal, "Código Duplicado", "Ya existe un producto registrado con ese código.");
            return;
        }

        // Crear ficha y agregar a bodega
        Producto nuevo(cod, nom, cat, precio, stock);
        if (gestor->agregarProducto(nuevo)) {
            modal.accept();
            actualizarTabla();
            QMessageBox::information(this, "Éxito", "Producto registrado en el inventario.");
        }
        });

    modal.exec();
}

// =====================================================
// BORRADO LÓGICO: ALTERNAR ACTIVO / DESACTIVADO
// =====================================================
void VentanaInventario::alternarEstadoSeleccionado() {
    int fila = tablaProductos->currentRow();
    if (fila < 0) {
        QMessageBox::warning(this, "Selección requerida", "Por favor seleccione un producto de la tabla.");
        return;
    }

    std::string codigo = tablaProductos->item(fila, 0)->text().toStdString();
    Producto* prod = gestor->buscarPorCodigo(codigo);

    if (prod != nullptr) {
        bool nuevoEstado = !prod->getActivo();
        gestor->cambiarEstado(codigo, nuevoEstado);
        actualizarTabla();

        QString mensaje = nuevoEstado ? "Producto reactivado correctamente." : "Producto desactivado (borrado lógico).";
        QMessageBox::information(this, "Estado Actualizado", mensaje);
    }
}

// =====================================================
// PERSISTENCIA: GUARDAR EN DISCO
// =====================================================
void VentanaInventario::guardarCambiosEnDisco() {
    if (gestor != nullptr && gestor->guardarEnArchivo("productos.json")) {
        QMessageBox::information(this, "Guardado", "El inventario se guardo correctamente en productos.json.");
    }
    else {
        QMessageBox::critical(this, "Error", "No se pudo guardar el archivo JSON de inventario.");
    }
}