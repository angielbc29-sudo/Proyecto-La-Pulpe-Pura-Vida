#include "Producto.h"

Producto::Producto(std::string codigo, std::string nombre, std::string categoria, double precio, int stock)
	: codigo(codigo), nombre(nombre), categoria(categoria), precio(precio), stock(stock), activo(true) 
{
	if (!validarPrecio(precio)) {
		this->precio = 0.0; 
	}
	if (!validarStock(stock)) {
		this->stock = 0;
	}

}

//getters

std::string Producto::getCodigo() const {
	return codigo;
}
std::string Producto::getNombre() const {
	return nombre;
}
std::string Producto::getCategoria() const {
	return categoria;
}
double Producto::getPrecio() const {
	return precio;
}
int Producto::getStock() const {
	return stock;
}
bool Producto::getActivo() const {
	return activo;
}

//validaciones 

bool Producto::validarPrecio(double nuevoPrecio) const {
	return nuevoPrecio > 0.0;
}
bool Producto::validarStock(int cantidad) const {
	return cantidad >= 0;
}

//modificaciones 

bool Producto::setPrecio(double nuevoPrecio) {
	if (validarPrecio(nuevoPrecio)) {
		precio = nuevoPrecio;
		return true;
	}
	return false;
}

bool Producto::agregarStock(int cantidad) {
	if (validarStock(cantidad)) {
		stock += cantidad;
		return true;
	}
	return false;
}

bool Producto::reducirStock(int cantidad) {
	if (cantidad >0 && stock >= cantidad) {
		stock -= cantidad;
		return true;
	}
	return false;
}

void Producto::setActivo(bool estado) {
	activo = estado;
}
