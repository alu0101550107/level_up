import QtQuick
import QtQuick.Controls
import LevelUp

ApplicationWindow {
    id: window
    visible: true
    width: 400
    height: 720
    title: "Level Up"
    color: Theme.background

    // Tocar una celda del mes cambia la fecha de Dia (eso ya lo hace
    // AppBackend en C++, ver el connect en AppBackend.cpp) y ademas
    // cambia de pestaña -- eso es puramente de UI, vive aqui.
    Connections {
        target: backend.month
        function onDateActivated(isoDate) {
            tabBar.currentIndex = 0
        }
    }

    AccentColorPicker {
        id: accentPicker
        parent: window.contentItem
        onAccepted: Theme.accent = pickedColor
    }

    header: ToolBar {
        background: Rectangle { color: Theme.background }
        contentHeight: 44

        Label {
            anchors.centerIn: parent
            text: swipeView.currentIndex === 0 ? "Día" : "Mes"
            color: Theme.textPrimary
            font.bold: true
        }

        ToolButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 4
            contentItem: Text {
                text: "●"
                color: Theme.accent
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle { color: "transparent" }
            onClicked: accentPicker.open()
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        DayPage {}
        MonthPage {}
    }

    // "+" flotante: crea un evento nuevo (semanal o puntual) desde
    // cualquiera de las dos pestañas. Reutiliza AddEventDialog en modo
    // creacion (editingEventId queda en -1 por defecto).
    RoundButton {
        id: addButton
        parent: window.contentItem
        z: 10
        width: 56
        height: 56
        radius: 28
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20

        contentItem: Text {
            text: "+"
            font.pixelSize: 28
            color: Theme.background
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 28
            color: Theme.accent
        }

        onClicked: addDialog.openForCreate()
    }

    AddEventDialog {
        id: addDialog
        parent: window.contentItem
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        background: Rectangle { color: Theme.surface }

        TabButton {
            contentItem: Text {
                text: "Día"
                color: tabBar.currentIndex === 0 ? Theme.accent : Theme.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle { color: "transparent" }
        }
        TabButton {
            contentItem: Text {
                text: "Mes"
                color: tabBar.currentIndex === 1 ? Theme.accent : Theme.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle { color: "transparent" }
        }
    }
}
