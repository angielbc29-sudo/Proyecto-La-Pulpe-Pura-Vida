#include <QApplication>
#include <QEventLoop>

#include "Bitacora.h"
#include "LoginDialog.h"
#include "MenuPrincipalView.h"
#include "ServicioAutenticacion.h"
#include "UsuarioJsonRepository.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Se crea el repositorio (si usuarios.json no existe, lo crea con los usuarios de prueba)
    UsuarioJsonRepository repo("usuarios.json");
    ServicioAutenticacion servicio(&repo);

    // Se repite: login -> menú -> (cerrar sesión) -> login -> ...
    while (true)
    {
        LoginDialog login(&servicio);
        if (login.exec() != QDialog::Accepted)
        {
            break;   // cerró el login sin entrar: el programa termina
        }

        // El menú se muestra SIN bloquear las demás ventanas (como Inventario),
        // y aquí se espera hasta que el menú se cierre.
        MenuPrincipalView menu;
        QEventLoop espera;
        QObject::connect(&menu, &QDialog::finished, &espera, &QEventLoop::quit);
        menu.show();
        espera.exec();

        if (menu.result() != QDialog::Accepted)
        {
            // Cerró la ventana con la X: también queda en la bitácora, y el programa termina
            Bitacora::registrar("Sesión", "Cierre del programa");
            break;
        }
        // Si presionó "Cerrar Sesión", el menú termina con Accepted y se vuelve al login
    }

    return 0;
}