#!/usr/bin/env bash
# level_up -- compila la app para Android y genera el APK.
#
# Automatiza lo repetible: encontrar el Qt para Android/NDK/JDK 17 ya
# instalados, configurar CMake, compilar, y generar el APK -- y
# opcionalmente instalarlo en un dispositivo conectado por USB con
# depuracion habilitada.
#
# Uso:
#   ./build_mobile.sh              # compila y genera el APK
#   ./build_mobile.sh --install    # ademas lo instala en el dispositivo conectado (adb)
#
# Variables de entorno para saltarse la autodeteccion:
#   QT_ANDROID_DIR   (p.ej. ~/Qt/6.11.1/android_arm64_v8a)
#   QT_HOST_DIR      (p.ej. ~/Qt/6.11.1/gcc_64)
#   ANDROID_SDK_ROOT (p.ej. ~/Android/Sdk)
#   ANDROID_NDK_ROOT (p.ej. ~/Android/Sdk/ndk/27.2.12479018)
#   JAVA17_HOME      (p.ej. /usr/lib/jvm/java-17-openjdk)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-android"
DO_INSTALL=0

for arg in "$@"; do
  case "$arg" in
    --install) DO_INSTALL=1 ;;
    *)
      echo "Argumento no reconocido: $arg (uso: $0 [--install])" >&2
      exit 1
      ;;
  esac
done

# --- Localizar Qt para Android + el kit de escritorio (QT_HOST_PATH) ---
if [ -z "${QT_ANDROID_DIR:-}" ]; then
  echo "==> Buscando una instalacion de Qt para Android en ~/Qt..."
  QT_ANDROID_DIR="$(find "$HOME/Qt" -maxdepth 1 -mindepth 1 -type d 2>/dev/null \
    | sort -t. -k1,1n -k2,2n -k3,3n \
    | while read -r d; do
        [ -f "$d/android_arm64_v8a/lib/cmake/Qt6/qt.toolchain.cmake" ] && echo "$d/android_arm64_v8a"
      done | tail -n1)"
fi
if [ -z "$QT_ANDROID_DIR" ] || [ ! -f "$QT_ANDROID_DIR/lib/cmake/Qt6/qt.toolchain.cmake" ]; then
  echo "No se encontro un kit de Qt para Android (arm64-v8a) en ~/Qt." >&2
  echo "Indicalo con: QT_ANDROID_DIR=/ruta/a/android_arm64_v8a $0" >&2
  exit 1
fi
QT_VERSION_DIR="$(dirname "$QT_ANDROID_DIR")"
echo "    Qt para Android: $QT_ANDROID_DIR"

if [ -z "${QT_HOST_DIR:-}" ]; then
  QT_HOST_DIR="$QT_VERSION_DIR/gcc_64"
fi
if [ ! -d "$QT_HOST_DIR" ]; then
  echo "No se encontro el kit de escritorio de Qt (QT_HOST_PATH) en $QT_HOST_DIR." >&2
  echo "Indicalo con: QT_HOST_DIR=/ruta/a/gcc_64 $0" >&2
  exit 1
fi
echo "    Qt de escritorio (QT_HOST_PATH): $QT_HOST_DIR"

# --- Localizar el Android SDK y NDK ---
ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}"
if [ ! -d "$ANDROID_SDK_ROOT" ]; then
  echo "No se encontro el Android SDK en $ANDROID_SDK_ROOT." >&2
  echo "Indicalo con: ANDROID_SDK_ROOT=/ruta/al/sdk $0" >&2
  exit 1
fi
echo "    Android SDK: $ANDROID_SDK_ROOT"

if [ -z "${ANDROID_NDK_ROOT:-}" ]; then
  ANDROID_NDK_ROOT="$(find "$ANDROID_SDK_ROOT/ndk" -maxdepth 1 -mindepth 1 -type d 2>/dev/null \
    | sort -V | tail -n1)"
fi
if [ -z "$ANDROID_NDK_ROOT" ] || [ ! -d "$ANDROID_NDK_ROOT" ]; then
  echo "No se encontro ningun NDK instalado en $ANDROID_SDK_ROOT/ndk/." >&2
  echo "Indicalo con: ANDROID_NDK_ROOT=/ruta/al/ndk/<version> $0" >&2
  exit 1
fi
echo "    Android NDK: $ANDROID_NDK_ROOT"

# --- Localizar JDK 17 (el AGP no funciona con JDK muy nuevo) ---
if [ -z "${JAVA17_HOME:-}" ]; then
  for candidate in /usr/lib/jvm/java-17-openjdk /usr/lib/jvm/java-17-openjdk-amd64 /usr/lib/jvm/temurin-17-jdk; do
    if [ -x "$candidate/bin/java" ]; then
      JAVA17_HOME="$candidate"
      break
    fi
  done
fi
if [ -z "${JAVA17_HOME:-}" ] || [ ! -x "$JAVA17_HOME/bin/java" ]; then
  echo "No se encontro un JDK 17 instalado (hace falta EXACTAMENTE para el paso de" >&2
  echo "empaquetado del APK -- un JDK mas nuevo rompe compileDebugJavaWithJavac)." >&2
  echo "Instalalo (Arch: sudo pacman -S jdk17-openjdk), o indicalo con:" >&2
  echo "    JAVA17_HOME=/ruta/al/jdk17 $0" >&2
  exit 1
fi
echo "    JDK 17 (para empaquetar): $JAVA17_HOME"

# --- Configurar ---
echo "==> Configurando el proyecto con CMake..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$QT_ANDROID_DIR/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DQT_HOST_PATH="$QT_HOST_DIR" \
  -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
  -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLEVELUP_BUILD_APP=ON

# --- Compilar + generar el APK ---
echo "==> Compilando levelup_app y generando el APK (puede tardar varios minutos)..."
JAVA_HOME="$JAVA17_HOME" cmake --build "$BUILD_DIR" --target levelup_app_make_apk -j"$(nproc)"

APK_PATH="$BUILD_DIR/app/android-build/build/outputs/apk/debug/android-build-debug.apk"
if [ ! -f "$APK_PATH" ]; then
  echo "La compilacion no genero el APK esperado en $APK_PATH" >&2
  exit 1
fi

echo ""
echo "Listo. APK generado en:"
echo "    $APK_PATH"

# --- Instalar en un dispositivo conectado (opcional) ---
if [ "$DO_INSTALL" = "1" ]; then
  ADB="$ANDROID_SDK_ROOT/platform-tools/adb"
  if [ ! -x "$ADB" ]; then
    echo "No se encuentra adb en $ADB, no se puede instalar automaticamente." >&2
    exit 1
  fi
  echo ""
  echo "==> Instalando en el dispositivo conectado..."
  DEVICE_COUNT="$("$ADB" devices | grep -c "device$" || true)"
  if [ "$DEVICE_COUNT" -lt 1 ]; then
    echo "No hay ningun dispositivo Android conectado y autorizado (revisa 'adb devices')." >&2
    exit 1
  fi
  "$ADB" install -r "$APK_PATH"
  echo "Instalado (-r conserva los datos si ya estaba instalada de antes)."
else
  echo ""
  echo "Para instalarlo en un dispositivo conectado por USB (con depuracion habilitada):"
  echo "    $0 --install"
  echo "o a mano:"
  echo "    $ANDROID_SDK_ROOT/platform-tools/adb install -r $APK_PATH"
fi
