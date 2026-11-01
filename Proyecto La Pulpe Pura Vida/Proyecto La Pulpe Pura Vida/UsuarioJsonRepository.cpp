#include "UsuarioJsonRepository.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "Administrador.h"
#include "Cajero.h"
#include "Turno.h"

//  Funciones auxiliares (solo se usan en este archivo) 

namespace
{
    // Hash SHA-256 de la contraseña de prueba "1234"
    const std::string HASH_PRUEBA = "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4";

    QString turnoATexto(Turno t)
    {
        switch (t)
        {
        case Turno::MANANA:   return "MANANA";
        case Turno::TARDE:    return "TARDE";
        case Turno::NOCHE:    return "NOCHE";
        case Turno::COMPLETO: return "COMPLETO";
        }
        return "MANANA";
    }

    Turno textoATurno(const QString& texto)
    {
        if (texto == "TARDE")    return Turno::TARDE;
        if (texto == "NOCHE")    return Turno::NOCHE;
        if (texto == "COMPLETO") return Turno::COMPLETO;
        return Turno::MANANA;
    }
}

// Constructor 

UsuarioJsonRepository::UsuarioJsonRepository(std::string ruta)
{
    this->rutaArchivo = ruta;

    // Si el archivo todavía no existe, se crea con los tres usuarios de prueba
    if (!QFile::exists(QString::fromStdString(rutaArchivo)))
    {
        std::vector<std::shared_ptr<Usuario>> iniciales;

        iniciales.push_back(std::make_shared<Administrador>(
            1, "carlosm", HASH_PRUEBA, "Carlos Mendoza", "1-1111-1111", "8888-1111", true));

        iniciales.push_back(std::make_shared<Cajero>(
            2, "anamaria", HASH_PRUEBA, "Ana María Gómez", "2-2222-2222", "8888-2222", true, Turno::MANANA));

        // Pedro tiene la cuenta inactiva (para probar el mensaje de acceso denegado)
        iniciales.push_back(std::make_shared<Cajero>(
            3, "pedror", HASH_PRUEBA, "Pedro Rojas", "3-3333-3333", "8888-3333", false, Turno::TARDE));

        serializar(iniciales);
    }
}

// Métodos públicos (contrato de IUsuarioRepository) 

std::vector<std::shared_ptr<Usuario>> UsuarioJsonRepository::obtenerTodos()
{
    return deserializar();
}

std::shared_ptr<Usuario> UsuarioJsonRepository::buscarPorUsername(std::string user)
{
    std::vector<std::shared_ptr<Usuario>> lista = deserializar();

    for (size_t i = 0; i < lista.size(); i++)
    {
        if (lista[i]->getUsername() == user)
        {
            return lista[i];
        }
    }
    return nullptr;   // no existe ese usuario
}

bool UsuarioJsonRepository::guardar(std::shared_ptr<Usuario> u)
{
    std::vector<std::shared_ptr<Usuario>> lista = deserializar();

    // No se permite repetir el nombre de usuario
    for (size_t i = 0; i < lista.size(); i++)
    {
        if (lista[i]->getUsername() == u->getUsername())
        {
            return false;
        }
    }

    lista.push_back(u);
    return serializar(lista);
}

bool UsuarioJsonRepository::actualizar(std::shared_ptr<Usuario> u)
{
    std::vector<std::shared_ptr<Usuario>> lista = deserializar();

    // Se busca el usuario por id y se reemplaza por la versión nueva
    for (size_t i = 0; i < lista.size(); i++)
    {
        if (lista[i]->getId() == u->getId())
        {
            lista[i] = u;
            return serializar(lista);
        }
    }
    return false;   // no se encontró ese id
}

// Métodos privados

bool UsuarioJsonRepository::serializar(std::vector<std::shared_ptr<Usuario>> lista)
{
    QJsonArray arreglo;

    for (size_t i = 0; i < lista.size(); i++)
    {
        std::shared_ptr<Usuario> u = lista[i];

        QJsonObject obj;
        obj["id"] = u->getId();
        obj["username"] = QString::fromStdString(u->getUsername());
        obj["passwordHash"] = QString::fromStdString(u->getPasswordHash());
        obj["nombreCompleto"] = QString::fromStdString(u->getNombreCompleto());
        obj["cedula"] = QString::fromStdString(u->getCedula());
        obj["telefono"] = QString::fromStdString(u->getTelefono());
        obj["activo"] = u->isActivo();
        obj["rol"] = QString::fromStdString(u->getNombreRol());

        // El turno solo existe en los cajeros
        std::shared_ptr<Cajero> cajero = std::dynamic_pointer_cast<Cajero>(u);
        if (cajero != nullptr)
        {
            obj["turno"] = turnoATexto(cajero->getTurno());
        }

        arreglo.append(obj);
    }

    QFile archivo(QString::fromStdString(rutaArchivo));
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    archivo.write(QJsonDocument(arreglo).toJson());
    archivo.close();
    return true;
}

std::vector<std::shared_ptr<Usuario>> UsuarioJsonRepository::deserializar()
{
    std::vector<std::shared_ptr<Usuario>> lista;

    QFile archivo(QString::fromStdString(rutaArchivo));
    if (!archivo.open(QIODevice::ReadOnly))
    {
        return lista;   // si no se puede abrir, se devuelve la lista vacía
    }

    QJsonDocument documento = QJsonDocument::fromJson(archivo.readAll());
    archivo.close();

    if (!documento.isArray())
    {
        return lista;
    }

    QJsonArray arreglo = documento.array();

    for (int i = 0; i < arreglo.size(); i++)
    {
        QJsonObject obj = arreglo[i].toObject();

        int id = obj["id"].toInt();
        std::string user = obj["username"].toString().toStdString();
        std::string hash = obj["passwordHash"].toString().toStdString();
        std::string nombre = obj["nombreCompleto"].toString().toStdString();
        std::string cedula = obj["cedula"].toString().toStdString();
        std::string telefono = obj["telefono"].toString().toStdString();
        bool activo = obj["activo"].toBool();
        QString rol = obj["rol"].toString();

        if (rol == "Cajero")
        {
            Turno turno = textoATurno(obj["turno"].toString());
            lista.push_back(std::make_shared<Cajero>(id, user, hash, nombre, cedula, telefono, activo, turno));
        }
        else
        {
            lista.push_back(std::make_shared<Administrador>(id, user, hash, nombre, cedula, telefono, activo));
        }
    }

    return lista;
}