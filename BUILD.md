# Compilar la app Android

Esta guia cubre, en dos partes:

1. **Instalacion del entorno** (Qt para Android, Android SDK/NDK, JDK) -- se
   hace una vez por maquina, es sobre todo interactiva (instaladores
   graficos), asi que no esta pensada para automatizarse del todo.
2. **Compilar y generar el APK** -- una vez el entorno esta listo, esto SI
   esta automatizado por `build_mobile.sh` (en la raiz del proyecto). Esta
   guia explica que hace ese script paso a paso, para poder reproducirlo a
   mano o diagnosticar un fallo.

Para iterar rapido sin tocar Android en absoluto, hay un build de
escritorio simulado (ver la seccion 2.1) que compila la misma app contra
Qt de escritorio.

---

## 1. Instalar el entorno (una sola vez)

### 1.1. Qt para Android

Se necesita el **Qt Online Installer** (cuenta gratuita de Qt Open Source
suficiente): <https://www.qt.io/download-qt-installer>

Al elegir componentes, dentro de la version de Qt que instales (probado con
Qt 6.11, pero cualquier 6.5+ deberia servir -- `qt_standard_project_setup(REQUIRES 6.5)`
en `app/CMakeLists.txt` es el minimo real) marca:

- **Android para arm64-v8a** (el kit de compilacion cruzada en si)
- El kit de **escritorio para Linux** correspondiente (p.ej. "Desktop gcc
  64-bit") -- hace falta como `QT_HOST_PATH`, Qt para Android necesita un
  Qt de escritorio ya compilado para generar codigo QML en tiempo de
  compilacion.

A diferencia de un proyecto con red (TLS), aqui **no** hace falta el
componente "Android OpenSSL": `level_up` no usa `QtNetwork` en absoluto,
todo es local (SQLite).

Instalacion tipica: `~/Qt/<version>/android_arm64_v8a/` (kit Android) y
`~/Qt/<version>/gcc_64/` (kit de escritorio, para `QT_HOST_PATH`).

### 1.2. Android SDK + NDK

El Qt Online Installer puede instalar Android Studio/el SDK por ti, o
puedes usar uno que ya tengas.

- **NDK**: Qt para Android suele fijar una version concreta de NDK
  compatible con esa version de Qt -- revisa la documentacion de Qt.
- **Android SDK** con la plataforma **android-34** o superior instalada
  (el proyecto compila contra `QT_ANDROID_TARGET_SDK_VERSION 34`, ver el
  comentario en `app/CMakeLists.txt`: SDK 35+ obliga al modo
  "edge-to-edge" sin gestion de insets implementada todavia).

Ubicacion tipica: `~/Android/Sdk`.

### 1.3. JDK -- **tiene que ser la version 17**

El Android Gradle Plugin (usado por Qt para empaquetar el APK) no funciona
con JDK muy nuevo. Si tu sistema tiene un JDK mas nuevo como version por
defecto (`java -version`), instala ademas el 17:

```bash
# Arch
sudo pacman -S jdk17-openjdk

# Debian/Ubuntu
sudo apt-get install openjdk-17-jdk
```

No hace falta cambiar el JDK por defecto del sistema -- `build_mobile.sh`
fija `JAVA_HOME` solo para el paso de empaquetado, sin tocar nada global.

### 1.4. Dispositivo o emulador para probar

Para instalar y probar en un movil real: activa "Opciones de
desarrollador" -> "Depuracion USB" en el telefono, conectalo por USB, y
autoriza el ordenador cuando el telefono lo pida. `adb devices` (dentro de
`<Android Sdk>/platform-tools/`) deberia listarlo como `device` (no
`unauthorized`).

---

## 2. Compilar

### 2.1. Build de escritorio (iteracion rapida, sin Android)

```bash
cmake -S . -B build-app -DLEVELUP_BUILD_APP=ON
cmake --build build-app
./build-app/app/levelup_app
```

### 2.2. Build de Android (APK)

```bash
./build_mobile.sh              # compila y genera el APK
./build_mobile.sh --install    # + lo instala en un dispositivo conectado por adb
```

Autodetecta el kit de Qt para Android, `QT_HOST_PATH`, el Android SDK/NDK,
y un JDK 17 -- si algo no esta donde el script espera, indicalo a mano
(ver los comentarios al principio de `build_mobile.sh` para las variables
de entorno).

El APK queda en:
```
build-android/app/android-build/build/outputs/apk/debug/android-build-debug.apk
```
