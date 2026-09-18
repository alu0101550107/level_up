import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LevelUp

// Selector visual de hora (rueda hora/minuto) en vez de escribir "HH:MM"
// a mano -- se abre desde AddEventDialog.qml tanto para la hora de inicio
// como la de fin (reutiliza la misma instancia, ver editingField alli).
Dialog {
    id: picker
    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 60 : 260, 260)

    signal timeChosen(string time)

    function openWithTime(hhmm) {
        if (hhmm && hhmm.length > 0) {
            var parts = hhmm.split(":");
            hourTumbler.currentIndex = parseInt(parts[0], 10);
            minuteTumbler.currentIndex = parseInt(parts[1], 10);
        } else {
            hourTumbler.currentIndex = 12;
            minuteTumbler.currentIndex = 0;
        }
        open();
    }

    onAccepted: {
        var hh = hourTumbler.currentIndex < 10 ? "0" + hourTumbler.currentIndex : "" + hourTumbler.currentIndex;
        var mm = minuteTumbler.currentIndex < 10 ? "0" + minuteTumbler.currentIndex : "" + minuteTumbler.currentIndex;
        timeChosen(hh + ":" + mm);
    }

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.accent
        border.width: 1
        radius: 6
    }

    header: Label {
        text: "Elegir hora"
        color: Theme.accent
        font.bold: true
        padding: 14
    }

    footer: RowLayout {
        width: picker.width
        spacing: 8

        Button {
            Layout.fillWidth: true
            Layout.margins: 8
            onClicked: picker.reject()
            contentItem: Text { text: "Cancelar"; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter }
            background: Rectangle {
                color: "transparent"
                border.color: Theme.textSecondary
                border.width: 1
                radius: 4
            }
        }
        Button {
            Layout.fillWidth: true
            Layout.margins: 8
            onClicked: picker.accept()
            contentItem: Text { text: "Aceptar"; color: Theme.background; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            background: Rectangle { color: Theme.accent; radius: 4 }
        }
    }

    RowLayout {
        width: picker.availableWidth
        height: 160
        spacing: 4

        Tumbler {
            id: hourTumbler
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: 24
            delegate: Text {
                text: modelData < 10 ? "0" + modelData : "" + modelData
                color: Theme.textPrimary
                font.pixelSize: Tumbler.displacement === 0 ? 22 : 16
                opacity: 1.0 - Math.min(Math.abs(Tumbler.displacement), 2) * 0.4
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
        Label {
            text: ":"
            color: Theme.accent
            font.bold: true
            font.pixelSize: 22
        }
        Tumbler {
            id: minuteTumbler
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: 60
            delegate: Text {
                text: modelData < 10 ? "0" + modelData : "" + modelData
                color: Theme.textPrimary
                font.pixelSize: Tumbler.displacement === 0 ? 22 : 16
                opacity: 1.0 - Math.min(Math.abs(Tumbler.displacement), 2) * 0.4
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
