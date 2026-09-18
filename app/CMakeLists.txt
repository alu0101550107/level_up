# Capa de conexion entre core/ (datos, sin Qt) y la interfaz Qt
# Quick/QML. Solo se compila con -DLEVELUP_BUILD_APP=ON (ver
# CMakeLists.txt raiz) para no obligar a tener Qt6 instalado solo para
# trabajar en core/.

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

# Sin esta llamada, qt6_add_qml_module no deja el modulo QML en un estado
# que Android pueda resolver en tiempo de ejecucion (mismo problema ya
# diagnosticado en crusader/mk1 y templar/mk3 -- "Module contains no type
# named Main" pese a que el .so enlaza bien).
qt_standard_project_setup(REQUIRES 6.5)

if(POLICY QTP0001)
  qt_policy(SET QTP0001 NEW)
endif()

qt6_add_executable(levelup_app
  src/main.cpp
  # EventBus.hpp no tiene un .cpp propio (es solo una senyal) -- hay que
  # listarlo explicitamente para que AUTOMOC lo procese, ya que por
  # defecto solo moquea cabeceras que comparten nombre con un .cpp
  # compilado del target.
  src/EventBus.hpp
  src/AppBackend.cpp
  src/DayController.cpp
  src/DayEventListModel.cpp
  src/MonthController.cpp
  src/MonthGridModel.cpp
  src/EditorController.cpp
)
target_link_libraries(levelup_app PRIVATE
  levelup_core
  Qt6::Quick
  Qt6::QuickControls2
)

if(ANDROID)
  # Manifest propio (android/AndroidManifest.xml) en vez del que genera Qt
  # por defecto -- solo para fijar el package id y el nombre visible de la
  # app. Sin permisos: la app no usa red, biometria ni notificaciones.
  set_target_properties(levelup_app PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    QT_ANDROID_TARGET_SDK_VERSION 34
  )
endif()

# El "pragma Singleton" dentro de Theme.qml no basta por si solo -- sin
# esta propiedad, qt6_add_qml_module lo trata como un tipo instanciable
# normal en vez de un singleton (mismo gotcha ya documentado en
# crusader/mk1/app/CMakeLists.txt).
set_source_files_properties(qml/theme/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE true)

qt6_add_qml_module(levelup_app
  URI LevelUp
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/DayPage.qml
    qml/MonthPage.qml
    qml/AddEventDialog.qml
    qml/TimePickerDialog.qml
    qml/AccentColorPicker.qml
    qml/theme/Theme.qml
  RESOURCES
    qml/assets/level_up_icon.png
)
