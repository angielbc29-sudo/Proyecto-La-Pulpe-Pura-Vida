#include "ProyectoLaPulpePuraVida.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ProyectoLaPulpePuraVida window;
    window.show();
    return app.exec();
}
