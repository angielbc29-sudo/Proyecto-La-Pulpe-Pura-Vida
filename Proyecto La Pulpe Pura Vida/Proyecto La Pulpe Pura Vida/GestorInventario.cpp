#include "GestorInventario.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

GestorInventario::GestorInventario() {

}

GestorInventario::~GestorInventario() {
	// Destructor vacío
}

//validaciones 

bool GestorInventario::validarCodigoUnico(const std::string& codigo) const {
	for (const auto& prod : listaProductos) {
		if (prod.getCodigo() == codigo) {
			return false;
		}
	}
	return true;
}

// agregar producto

bool GestorInventario::agregarProducto(const Producto& nuevo) {
	if (!validarCodigoUnico(nuevo.getCodigo())) {
		return false;
	}
	if (!nuevo.validarPrecio(nuevo.getPrecio())) {
		return false;
	}
	if (!nuevo.validarStock(nuevo.getStock())) {
		return false;
	}
	listaProductos.push_back(nuevo);
	return true;
}

//Buscar producto por codigo

Producto* GestorInventario::buscarPorCodigo(const std::string& codigo) {
	for (auto& prod : listaProductos) {
		if (prod.getCodigo() == codigo) {
			return &prod;
		}
	}
	return nullptr;
}

//actualizar nombre y precio 

bool GestorInventario::actualizarProducto(const std::string& codigo, const std::string& nombre, double precio) {
	Producto* prod = buscarPorCodigo(codigo);
	if (prod == nullptr || nombre.empty()) {
		return false;
	}
	if (!prod->validarPrecio(precio)) {
		return false;
	}
	bool estado = prod->getActivo();   // el constructor siempre crea activo = true
	*prod = Producto(prod->getCodigo(), nombre, prod->getCategoria(), precio, prod->getStock());
	prod->setActivo(estado);
	return true;
}

//cambiar estado (borrador logico)

bool GestorInventario::cambiarEstado(const std::string& codigo, bool activo) {
	Producto* prod = buscarPorCodigo(codigo);
	if (prod != nullptr) {
		prod->setActivo(activo);
		return true;
	}
	return false;

}

//Consultar productos activos

std::vector <Producto> GestorInventario::getProductosActivos() const {
	std::vector<Producto> activos;
	for (const auto& prod : listaProductos) {
		if (prod.getActivo()) {
			activos.push_back(prod);
		}
	}
	return activos;
}

//Metodos de conexion con el modulo de ventas

// Ventas pregunta: ¿Hay suficiente stock para el carrito?
bool GestorInventario::verificarDisponibilidad(const std::string& codigo, int cantidad) {

	Producto* prod = buscarPorCodigo(codigo);
	if (prod != nullptr && prod->getActivo() && prod->getStock() >= cantidad) {
		return true; 
	}
	return false;
}

// Ventas ordena: el cliente ya pago, rebaja las existencias del inventario. 

bool GestorInventario::descontarStock(const std::string& codigo, int cantidad) {
	Producto* prod = buscarPorCodigo(codigo);
	if (prod != nullptr && prod->getActivo()) {
		return prod->reducirStock(cantidad);
	}
	return false;
}

// =====================================================
// PERSISTENCIA OFICIAL: FORMATO JSON (Qt)
// =====================================================

// Guarda todos los productos en un archivo JSON ordenado
bool GestorInventario::guardarEnArchivo(const std::string& ruta) {
	QFile archivo(QString::fromStdString(ruta));
	if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}

	QJsonArray arrayProductos;
	for (const auto& prod : listaProductos) {
		QJsonObject objProd;
		objProd["codigo"] = QString::fromStdString(prod.getCodigo());
		objProd["nombre"] = QString::fromStdString(prod.getNombre());
		objProd["categoria"] = QString::fromStdString(prod.getCategoria());
		objProd["precio"] = prod.getPrecio();
		objProd["stock"] = prod.getStock();
		objProd["activo"] = prod.getActivo();

		arrayProductos.append(objProd);
	}

	QJsonDocument doc(arrayProductos);
	archivo.write(doc.toJson(QJsonDocument::Indented)); // Indented para que se lea ordenado
	archivo.close();
	return true;
}

// Lee el archivo JSON y reconstruye las fichas en la bodega
bool GestorInventario::cargarDesdeArchivo(const std::string& ruta) {
	QFile archivo(QString::fromStdString(ruta));
	if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false; // El archivo aun no existe (ej. primera ejecucion)
	}

	QByteArray datos = archivo.readAll();
	archivo.close();

	QJsonDocument doc = QJsonDocument::fromJson(datos);
	if (!doc.isArray()) {
		return false; // Archivo corrupto o no es lista JSON
	}

	listaProductos.clear(); // Limpiamos para no duplicar

	QJsonArray arrayProductos = doc.array();
	for (const auto& valor : arrayProductos) {
		QJsonObject objProd = valor.toObject();

		std::string cod = objProd["codigo"].toString().toStdString();
		std::string nom = objProd["nombre"].toString().toStdString();
		std::string cat = objProd["categoria"].toString().toStdString();
		double precio = objProd["precio"].toDouble();
		int stock = objProd["stock"].toInt();
		bool activo = objProd["activo"].toBool();

		Producto prod(cod, nom, cat, precio, stock);
		prod.setActivo(activo);
		listaProductos.push_back(prod);
	}

	return true;
}
