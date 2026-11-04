#pragma once
#include <QString>
#include <QChar>
#include <QtGlobal>

namespace Formato {

	inline QString sinSimbolo(double monto) {
		const qlonglong v = qRound64(monto);
		const QString digitos = QString::number(qAbs(v));

		QString resultado;

		for (int i = 0; i < digitos.size(); ++i) {
			if (i > 0 && (digitos.size() - i) % 3 == 0) resultado += QChar('.');
			resultado += digitos[i];

		}

		return (v < 0 ? QString("-") : QString()) + resultado;
	}

	inline QString colones(double monto) {
		return QString(QChar(0x20A1)) + sinSimbolo(monto);

	}

	inline QString numero(int n) {
		return QString::number(n).rightJustified(4, QChar('0'));

	}
	inline QString centrar(const QString& texto, int ancho) {
		const int espacios = qMax(0, (ancho - static_cast<int>(texto.size())) / 2);

		return QString(espacios, QChar(' ')) + texto;
	}

	inline QString fila(const QString& izq, const QString& der, int ancho) {
		const int espacios = qMax(1, ancho - static_cast<int>(izq.size() + der.size()));
		return izq + QString(espacios, QChar(' ')) + der;

	}

}
