#pragma once
#include <string>
#include<vector>
#include"Producto.h"


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

};