#include "GestorInventario.h"

GestorInventario::GestorInventario() {

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
	if (prod != nullptr) {
		return prod->setPrecio(precio);
	}
	return false;
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