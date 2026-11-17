# La Pulpe Pura Vida

Aplicación de escritorio en **C++17 con Qt 6 (Qt Widgets)** que digitaliza la operación diaria de una pulpería costarricense de barrio: inventario, ventas, comprobantes y control de usuarios.

Proyecto final del curso **EIF201 - Programación I** (II Ciclo 2026), Universidad Nacional de Costa Rica, Sede Regional Chorotega - Campus Nicoya. Profesor: M.S. Alex Villegas Carranza.

## El problema

Muchas pulperías todavía llevan el negocio en cuadernos y calculadora. Por eso:

- los productos se agotan sin que el dueño se dé cuenta a tiempo;
- hay errores al cobrar y al calcular el vuelto;
- no queda un historial confiable de las ventas;
- no se sabe quién hizo cada cosa en la caja.

## La solución

El sistema se divide en cuatro módulos, uno por integrante, que se abren desde un mismo menú principal según el rol de quien inició sesión.

| # | Módulo | Qué hace | Responsable |
|---|--------|----------|-------------|
| 1 | **Productos e Inventario** | Catálogo visual tipo «Estante Vivo» con tarjetas, buscador, filtros (todos, por reponer, inactivos), alertas de poco stock o agotado, registrar, editar, reponer, desactivar y reactivar productos. | Tifany Brais |
| 2 | **Ventas y Cobro** | Terminal de caja: buscar productos, armar el carrito, calcular subtotal, IVA y total, recibir el efectivo, dar el vuelto y descontar el stock. | Berny Arias |
| 3 | **Comprobantes e Historial** | Historial permanente de ventas con búsqueda y filtro por fechas, detalle de cada venta y generación del ticket o factura en **PDF, TXT o HTML**. | Anyoleth González |
| 4 | **Usuarios y Permisos** | Inicio de sesión con contraseña cifrada (SHA-256), roles con permisos, gestión de empleados (registrar, activar y desactivar) y **Bitácora del Sistema**, donde queda anotado quién hizo cada acción importante. | Angie Bustos |

### Roles

| Rol | Puede usar |
|-----|-----------|
| **Administrador** | Los 5 módulos: Inventario, Ventas, Historial, Usuarios y Bitácora. |
| **Cajero** | Ventas y Cobro, y Comprobantes e Historial. Tiene turno (mañana, tarde, noche o completo). |
| **Auditor** | Solo la Bitácora del Sistema, para revisar lo que pasó sin poder cambiar nada. |

## Tecnologías

- **C++17** y **Qt 6.12** (Qt Widgets; la interfaz se arma por código).
- **Visual Studio** con la extensión **Qt VS Tools** (toolset v145, x64).
- Persistencia en archivos **JSON** con `QJsonDocument`.
- Contraseñas cifradas con `QCryptographicHash` (SHA-256).
- PDF con `QPdfWriter` y `QTextDocument`.
- Punteros inteligentes (`std::shared_ptr`, `std::unique_ptr`) y contenedores de la STL.

## Diseño

- **Capas:** las pantallas (presentación) solo muestran y capturan datos; la lógica va en clases como `ServicioAutenticacion`, `GestorInventario`, `VentasCobro` o `Historial`; el guardado en disco va en repositorios o almacenes (`UsuarioJsonRepository`, `AlmacenJson`).
- **Herencia y polimorfismo:** `Usuario` → `Administrador`, `Cajero` y `Auditor`, cada uno con su `tienePermiso()`; `Ticket` → `TicketTxt`, `TicketHtml` y `TicketPdf`.
- **Interfaces:** `IUsuarioRepository` y `Almacen`, para poder cambiar la forma de guardar sin tocar la lógica.
- **Singleton:** `Sesion` guarda quién inició sesión, así Ventas y la Bitácora saben quién está trabajando.
- **Principio abierto/cerrado:** el rol Auditor y el formato PDF se agregaron como clases nuevas, sin modificar el login ni el historial.

## Cómo ejecutarlo

1. Instalar **Visual Studio** con el desarrollo de escritorio en C++ y **Qt 6.12 (msvc2022_64)**.
2. Instalar la extensión **Qt VS Tools** y registrar esa versión de Qt con el nombre `6.12.0_msvc2022_64`.
3. Clonar el repositorio:
   ```bash
   git clone https://github.com/angielbc29-sudo/Proyecto-La-Pulpe-Pura-Vida.git
   ```
4. Abrir `Proyecto La Pulpe Pura Vida.slnx`, elegir **Debug | x64** y ejecutar (F5).

Los archivos de datos se leen de la carpeta del proyecto:

| Archivo | Contenido |
|---------|-----------|
| `usuarios.json` | Empleados (si no existe, se crea solo con los usuarios de prueba). |
| `productos.json` | Catálogo del inventario. |
| `ventas.json` | Ventas realizadas (se crea solo). |
| `bitacora.json` | Registro de actividad (se crea solo). |
| `tickets/` | Comprobantes generados en PDF, TXT o HTML. |

### Usuarios de prueba

Todos tienen la contraseña **`1234`**.

| Usuario | Nombre | Rol | Estado |
|---------|--------|-----|--------|
| `carlosm` | Carlos Mora | Administrador | Activo |
| `anamaria` | Ana Maria Solis | Cajero (mañana) | Activo |
| `pedror` | Pedro Rojas | Cajero (tarde) | Inactivo (para probar el rechazo del login) |
| `laurav` | Laura Vargas | Auditor | Activo |

## Equipo

| Integrante | Módulo | GitHub |
|------------|--------|--------|
| Tifany Cecilia Brais Villegas | 1. Productos e Inventario | [tifanybrais-lab](https://github.com/tifanybrais-lab) |
| Berny Antonio Arias López | 2. Ventas y Cobro | [berny-arias](https://github.com/berny-arias) |
| Anyoleth De Los Ángeles González Parra | 3. Comprobantes e Historial | [anygonzalez3008-dev](https://github.com/anygonzalez3008-dev) |
| Angie Lisseth Bustos Castillo | 4. Usuarios y Permisos | [angielbc29-sudo](https://github.com/angielbc29-sudo) |

**Organización del trabajo:** cada integrante trabajó en su propia rama y los cambios se unieron a `main` mediante pull requests. Las tareas se organizaron en [Trello](https://trello.com/b/w0D3sCpT/proyecto-la-pulpe) y la comunicación fue por WhatsApp y Discord. Prototipo de diseño: [la-pulpe-pura-vida-1.vercel.app](https://la-pulpe-pura-vida-1.vercel.app/).
