#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProyectoLaPulpePuraVida.h"

class ProyectoLaPulpePuraVida : public QMainWindow
{
    Q_OBJECT

public:
    ProyectoLaPulpePuraVida(QWidget *parent = nullptr);
    ~ProyectoLaPulpePuraVida();

private:
    Ui::ProyectoLaPulpePuraVidaClass ui;
};

