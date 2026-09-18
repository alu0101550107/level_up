import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LevelUp

// Selector de color propio -- deliberadamente NO usa ColorDialog
// (QtQuick.Dialogs): ese componente no expone background/contentItem para
// re-estilarlo, y forzarlo rompería el aspecto negro/accent del resto de
// la app. Adaptado de crusader/mk1/app/qml/ColorPickerDialog.qml.
Dialog {
    id: picker
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 300, 300)

    // Color de entrada/salida: se fija antes de abrir (Theme.accent) y se
    // lee tras aceptar (ver Main.qml).
    property color pickedColor: Theme.accent

    // Copia de trabajo que mueven los deslizadores -- pickedColor no
    // cambia hasta pulsar "Aplicar", para poder cancelar sin dejar rastro.
    property real r: 0
    property real g: 0
    property real b: 0

    onOpened: {
        r = pickedColor.r;
        g = pickedColor.g;
        b = pickedColor.b;
    }

    onAccepted: pickedColor = Qt.rgba(r, g, b, 1.0)

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.accent
        border.width: 1
        radius: 6
    }

    header: Label {
        text: "Color de acento"
        color: Theme.accent
        font.bold: true
        padding: 14
    }

    footer: RowLayout {
        width: picker.width
        spacing: 8

        Button {
            id: cancelButton
            Layout.fillWidth: true
            Layout.margins: 8
            onClicked: picker.reject()
            contentItem: Text {
                text: "Cancelar"
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle {
                color: "transparent"
                border.color: Theme.textSecondary
                border.width: 1
                radius: 4
            }
        }
        Button {
            id: acceptButton
            Layout.fillWidth: true
            Layout.margins: 8
            onClicked: picker.accept()
            contentItem: Text {
                text: "Aplicar"
                color: Theme.background
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle {
                color: Theme.accent
                radius: 4
            }
        }
    }

    ColumnLayout {
        width: picker.availableWidth
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 4
            color: Qt.rgba(picker.r, picker.g, picker.b, 1.0)
            border.color: Theme.accent
            border.width: 1
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: "R"; color: Theme.textPrimary }
            Slider {
                from: 0
                to: 1
                value: picker.r
                Layout.fillWidth: true
                onMoved: picker.r = value
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label { text: "G"; color: Theme.textPrimary }
            Slider {
                from: 0
                to: 1
                value: picker.g
                Layout.fillWidth: true
                onMoved: picker.g = value
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label { text: "B"; color: Theme.textPrimary }
            Slider {
                from: 0
                to: 1
                value: picker.b
                Layout.fillWidth: true
                onMoved: picker.b = value
            }
        }
    }
}
