pragma Singleton
import QtQuick
import Qt.labs.settings

// Fondo/texto se quedan fijos (negro minimalista) -- solo `accent` es
// configurable por el usuario, persistido via Qt.labs.settings (mismo
// mecanismo que crusader/mk1/app/qml/theme/Theme.qml, sin cambios de
// CMake: viene con Qt6::Quick).
Item {
    readonly property color defaultAccent: "#3B82F6"

    readonly property color background: "#05070a"
    readonly property color surface: "#12161f"
    readonly property color surfaceAlt: "#1b202c"
    readonly property color textPrimary: "#e8ecf5"
    readonly property color textSecondary: "#8792a6"

    property alias accent: settings.accent

    Settings {
        id: settings
        category: "Theme"
        property color accent: defaultAccent
    }
}
