cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
project(level_up CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra)

if(NOT ANDROID)
  enable_testing()
endif()

# A diferencia de templar (donde sqlite solo hace falta bajo
# TEMPLAR_BUILD_PHONE=ON), aqui se añade siempre: core/ es el unico
# consumidor de SQLite del proyecto y tiene que compilar y testear
# standalone via ctest sin depender de Qt.
add_subdirectory(third_party/sqlite3)
add_subdirectory(core)

# Apagado por defecto para no obligar a quien solo quiera trabajar en
# core/ (datos, sin Qt) a tener Qt6 instalado -- se activa a mano con
# -DLEVELUP_BUILD_APP=ON, o lo hace build_mobile.sh automaticamente para
# el build de Android.
option(LEVELUP_BUILD_APP "Compilar la app Qt Quick (escritorio o Android)" OFF)

if(LEVELUP_BUILD_APP)
  add_subdirectory(app)
endif()
