#pragma once
#include <vector>
#include "VentasCobro.h"   // [VENTAS] 

class Almacen {
public:
    virtual ~Almacen() = default;
    virtual bool guardar(const VentasCobro& venta) = 0;
    virtual std::vector<VentasCobro> listar() const = 0;
    virtual VentasCobro buscar(int numero) const = 0;
    virtual bool existe(int numero) const = 0;
};
