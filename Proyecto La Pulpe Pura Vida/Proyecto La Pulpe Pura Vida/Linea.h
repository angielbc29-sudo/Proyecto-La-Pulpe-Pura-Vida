#pragma once

#include <QString>

class Linea
{
public:
	Linea() = default;

	Linea(const QString& nombre, int cantidad, double precio)
		: nombre_(nombre), cantidad_(cantidad), precio_(precio)
	{
	}

	const QString& nombre() const { return nombre_; }
	int cantidad() const { return cantidad_; }
	double precio() const { return precio_; }
	double subtotal() const { return cantidad_ * precio_; }

private:
	QString nombre_;
	int cantidad_ = 0;
	double precio_ = 0.0;
};
