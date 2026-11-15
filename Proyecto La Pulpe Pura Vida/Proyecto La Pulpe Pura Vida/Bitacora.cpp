#include "Bitacora.h"

#include <memory>

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "Sesion.h"
#include "Usuario.h"

// ---------------- Funciones auxiliares (solo se usan en este archivo) ----------------

namespace
{
    const QString RUTA_ARCHIVO = "bitacora.json";

    // Lee el arreglo guardado en el archivo; si no existe o está dañado, devuelve uno vacío
    QJsonArray leerArreglo()
    {
        QFile archivo(RUTA_ARCHIVO);
        if (!archivo.open(QIODevice::ReadOnly))
        {
            return QJsonArray();
        }

        QJsonDocument documento = QJsonDocument::fromJson(archivo.readAll());
        archivo.close();

        if (!documento.isArray())
        {
            return QJsonArray();
        }
        return documento.array();
    }
}

// ---------------- Métodos ----------------

void Bitacora::registrar(std::string modulo, std::string accion, std::string detalle)
{
    // El usuario y el rol se toman de la sesión; si nadie ha iniciado sesión se indica así
    std::string usuario = "Sin sesión";
    std::string rol = "-";

    std::shared_ptr<Usuario> actual = Sesion::getInstancia().getUsuarioActual();
    if (actual != nullptr)
    {
        usuario = actual->getUsername();
        rol = actual->getNombreRol();
    }

    QJsonObject obj;
    obj["fechaHora"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    obj["usuario"] = QString::fromStdString(usuario);
    obj["rol"] = QString::fromStdString(rol);
    obj["modulo"] = QString::fromStdString(modulo);
    obj["accion"] = QString::fromStdString(accion);
    obj["detalle"] = QString::fromStdString(detalle);

    QJsonArray arreglo = leerArreglo();
    arreglo.append(obj);

    QFile archivo(RUTA_ARCHIVO);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return;
    }

    archivo.write(QJsonDocument(arreglo).toJson());
    archivo.close();
}

std::vector<RegistroBitacora> Bitacora::obtenerTodos()
{
    std::vector<RegistroBitacora> lista;

    QJsonArray arreglo = leerArreglo();

    for (int i = 0; i < arreglo.size(); i++)
    {
        QJsonObject obj = arreglo[i].toObject();

        RegistroBitacora registro;
        registro.fechaHora = obj["fechaHora"].toString().toStdString();
        registro.usuario = obj["usuario"].toString().toStdString();
        registro.rol = obj["rol"].toString().toStdString();
        registro.modulo = obj["modulo"].toString().toStdString();
        registro.accion = obj["accion"].toString().toStdString();
        registro.detalle = obj["detalle"].toString().toStdString();

        lista.push_back(registro);
    }

    return lista;
}
