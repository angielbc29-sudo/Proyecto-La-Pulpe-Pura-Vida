#pragma once
#include <string>
#include <vector>


//clase 1: Producto (Ficha individual)

class Producto {

private : 
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
	std::string getCatgoria() const; 
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

};//fin de la clase Producto


//Clase 2: Gestor Inventario (El archhivador)

class GestorInventario {

private: 
	std::vector <Producto> listaProductos; //vector de productos

public: 
	//Contructor y destructor 
	GestorInventario(); 
	~GestorInventario();

	//Metodos del inventario 
	bool agregarProducto(const Producto& nuevo);
	Producto* buscarPorCodigo(const std::string& codigo);
	bool actualizarProducto(const std::string& codigo, const std::string& nombre, double precio);
	bool cambiarEstado(const std::string& codigo, bool activo);

	//Metodos de conexion para ventas

	bool verificarDisponibilidad(const std::string& codigo, int cantidad);
	bool descontarStock(const std::string& codigo, int cantidad); 

	//Consulta y validaciones de unicidad

	std::vector<Producto> getProductosActivos() const; 
	bool validarCodigoUnico(const std::string& codigo) const; 

};//fin de la clase GestorInventario