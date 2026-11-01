#include <QApplication>

#include "LoginDialog.h"
#include "MenuPrincipalView.h"
#include "ServicioAutentication.h"
#include "UsuarioJsonRepository.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Se crea el repositorio (si usuarios.json no existe, lo crea con los usuarios de prueba)
    UsuarioJsonRepository repo("usuarios.json");
    ServicioAutentication servicio(&repo);

    // Primero se muestra el inicio de sesión
    LoginDialog login(&servicio);
    if (login.exec() != QDialog::Accepted)
    {
        return 0;   // si cierra el login sin entrar, el programa termina
    }

    // Si las credenciales fueron correctas, se muestra el menú principal
    MenuPrincipalView menu;
    menu.show();

    return app.exec();
}