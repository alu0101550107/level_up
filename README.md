# Level Up

Organizador diario minimalista para Android, escrito en C++/Qt 6. Sin
servidor, sin red, sin notificaciones (con la capa de Xiaomi encima de
Android, hacerlas fiables es una batalla perdida) -- todo vive en una base
de datos SQLite local, en el almacenamiento privado de la app.

## Modelo de datos

Dos tipos de eventos, sobre una misma tabla `events`:

- **Recurrentes**: una plantilla por dia de la semana ("todos los lunes a
  las 7, gimnasio"), que se repite cada semana.
- **Puntuales**: un evento para una fecha exacta, una sola vez.

Marcar un evento como "hecho" es **por fecha concreta**: completar el
"gimnasio" el lunes 21 no lo marca como hecho el lunes 28 (misma
plantilla, distinta ocurrencia). Ver el esquema completo y el porque de
cada decision en `core/src/Database.cpp` (`migrate()`).

## Estructura

```
core/     Datos (SQLite vendorizado, EventRepository) -- sin Qt, testeable con ctest
app/      Interfaz Qt Quick/QML (Dia, Mes, dialogo de alta/edicion)
third_party/
  sqlite3/   Amalgamation de SQLite vendorizada (Android no trae libsqlite3.so estable)
```

## Interfaz

- **Día**: timeline vertical -- 00:00 abajo, 24:00 arriba, eventos como
  cajas colgando de una linea central. Mantener pulsada una caja abre
  "Editar"/"Borrar".
- **Mes**: calendario del mes, un punto por dia con eventos (relleno si
  ya estan todos hechos). Tocar un dia lleva a Día con esa fecha.
- Un "+" flotante abre el dialogo para crear un evento (semanal o
  puntual) desde cualquiera de las dos pantallas.
- El color de acento (azul por defecto) se puede cambiar desde el icono
  del header -- queda guardado entre arranques.

## Compilar

**Núcleo (sin Qt, escritorio):**
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

**App completa (requiere Qt6, `find_package` la resuelve por pkg-config en escritorio):**
```bash
cmake -S . -B build-app -DLEVELUP_BUILD_APP=ON
cmake --build build-app
./build-app/app/levelup_app
```

**Android** (requiere Qt para Android, Android SDK/NDK, JDK 17 -- ver `BUILD.md`):
```bash
./build_mobile.sh              # compila y genera el APK
./build_mobile.sh --install    # + lo instala en un dispositivo conectado por adb
```
