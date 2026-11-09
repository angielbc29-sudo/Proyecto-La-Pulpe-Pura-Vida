#pragma once
#include <stdexcept>
#include <QString>

class ErrorHistorial : public std::runtime_error {
public:
	explicit ErrorHistorial(const QString& mensaje)
		: std::runtime_error(mensaje.toStdString()) {}
};

