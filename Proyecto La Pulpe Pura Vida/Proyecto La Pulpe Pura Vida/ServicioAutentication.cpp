#include "ServicioAutentication.h"

#include <QByteArray>
#include <QCryptographicHash>

// ---------------- Constructor ----------------

ServicioAutenticacion::ServicioAutenticacion(IUsuarioRepository* repo)
{
    this->repo = repo;
}

// ---------------- Métodos ----------------

ResultadoLogin ServicioAutenticacion::autenticar(std::string user, std::string passPlano,
    std::shared_ptr<Usuario>& usuarioOut)
{
    usuarioOut = nullptr;

    // 1. ¿Existe el usuario?
    std::shared_ptr<Usuario> usuario = repo->buscarPorUsername(user);
    if (usuario == nullptr)
    {
        return ResultadoLogin::USUARIO_NO_EXISTE;
    }

    // 2. ¿La contraseña es correcta?
    if (!verificarHash(passPlano, usuario->getPasswordHash()))
    {
        return ResultadoLogin::CREDENCIALES_INVALIDAS;
    }

    // 3. ¿La cuenta está activa?
    //    (se revisa después de la contraseña para no revelar el estado a quien no la conoce)
    if (!usuario->isActivo())
    {
        return ResultadoLogin::CUENTA_DESACTIVADA;
    }

    usuarioOut = usuario;
    return ResultadoLogin::EXITOSO;
}

bool ServicioAutenticacion::verificarHash(std::string passPlano, std::string hashGuardado)
{
    // Se cifra la contraseña escrita con SHA-256 y se pasa a texto hexadecimal
    QByteArray hash = QCryptographicHash::hash(
        QByteArray::fromStdString(passPlano), QCryptographicHash::Sha256).toHex();

    return hash.toStdString() == hashGuardado;
}