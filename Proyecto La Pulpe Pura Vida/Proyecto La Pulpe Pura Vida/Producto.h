#pragma once
#include <string>
class Producto {

private:
	std::string codigo; //revisar en ventas
	std::string nombre;
	std::string categoria;
	double precio;
	int stock; //revisar en ventas
	bool activo;

public:
	//Constructor
	Producto(std::string codigo, std::string nombre, std::string categoria, double precio, int stock);

	//getters (Para consultar datos)
	std::string getCodigo() const;
	std::string getNombre() const;
	std::string getCategoria() const;
	double getPrecio() const;
	int getStock() const;
	bool getActivo() const;

	//Metodos de negocio y setters con validaciones 

	bool setPrecio(double nuevoPrecio);
	bool agregarStock(int cantidad);
	bool reducirStock(int canidad);
	void setActivo(bool estado);

	//Validaciones explicitas de POO

	bool validarPrecio(double nuevoPrecio) const;
	bool validarStock(int cantidad) const;

};
