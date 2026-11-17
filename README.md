# 🛒 La Pulpe Pura Vida

Proyecto final del curso **EIF201 - Programación I** (II Ciclo 2026), Universidad Nacional de Costa Rica, Sede Regional Chorotega - Campus Nicoya.

## El problema

Muchas pulperías costarricenses de barrio todavía llevan su negocio en cuadernos y con calculadora. Por eso los productos se agotan sin que el dueño se dé cuenta a tiempo, hay errores al cobrar y al dar el vuelto, no queda un historial confiable de las ventas y no se sabe quién hizo cada cosa en la caja.

## La solución

**La Pulpe Pura Vida** es una aplicación de escritorio hecha en **C++17 con Qt 6** que digitaliza el trabajo diario de la pulpería. Cada persona entra con su usuario y contraseña, y el sistema le muestra solo los módulos que le corresponden según su rol. Toda la información se guarda en archivos JSON, así que no se pierde al cerrar el programa.

## Módulos

| Módulo | Qué hace |
|--------|----------|
| 📦 **Productos e Inventario** | Muestra el catálogo como un «Estante Vivo» con tarjetas. Permite registrar, editar, reponer, desactivar y reactivar productos, buscarlos y filtrarlos, y avisa cuando un producto tiene poco stock o se agotó. |
| 💰 **Ventas y Cobro** | Terminal de caja: busca productos, arma el carrito, calcula subtotal, IVA y total, recibe el efectivo, calcula el vuelto y descuenta el inventario al cobrar. |
| 🧾 **Comprobantes e Historial** | Guarda todas las ventas, permite buscarlas y filtrarlas por fecha, ver el detalle de cada una y generar el ticket o la factura en PDF, TXT o HTML. |
| 👥 **Usuarios y Permisos** | Inicio de sesión con contraseña cifrada, roles con permisos (Administrador, Cajero y Auditor), registro y activación o desactivación de empleados, y una **Bitácora del Sistema** donde queda anotado quién hizo cada acción. |

**Roles:**
- **Administrador:** puede usar todos los módulos.
- **Cajero:** solo puede usar Ventas y el Historial de comprobantes.
- **Auditor:** solo puede consultar la Bitácora del Sistema.

## Cómo probarlo

1. Tener **Visual Studio** con desarrollo de escritorio en C++, **Qt 6.12 (msvc2022_64)** y la extensión **Qt VS Tools**.
2. Clonar el repositorio:
   ```bash
   git clone https://github.com/angielbc29-sudo/Proyecto-La-Pulpe-Pura-Vida.git
   ```
3. Abrir `Proyecto La Pulpe Pura Vida.slnx`, elegir **Debug | x64** y ejecutar con **F5**.
4. Iniciar sesión con alguno de estos usuarios de prueba (todos tienen la contraseña **`1234`**):

| Usuario | Rol | Para probar |
|---------|-----|-------------|
| `carlosm` | Administrador | Todos los módulos. |
| `anamaria` | Cajero | Ventas e Historial. |
| `laurav` | Auditor | La Bitácora del Sistema. |
| `pedror` | Cajero (inactivo) | Que el sistema no deje entrar a una cuenta desactivada. |

## Equipo

| Integrante | Módulo |
|------------|--------|
| Tifany Brais | Productos e Inventario |
| Berny Arias | Ventas y Cobro |
| Anyoleth González | Comprobantes e Historial |
| Angie Bustos | Usuarios y Permisos |
