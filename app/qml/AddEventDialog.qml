import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LevelUp

// Un unico dialogo para crear Y editar -- evita duplicar un
// EditEventDialog.qml casi identico. En modo edicion (editingEventId >= 0)
// se ocultan el switch semanal/puntual y el selector de dia/fecha: cambiar
// el tipo de recurrencia no lo soporta EventRepository::updateEvent (solo
// toca titulo/hora, ver core/src/EventRepository.cpp) -- cambiarlo de
// verdad es borrar+recrear.
Dialog {
    id: dialog
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 32 : 320, 340)

    property int editingEventId: -1
    property bool isWeekly: true

    readonly property var weekdayNames: [
        "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"
    ]

    function openForCreate() {
        editingEventId = -1;
        isWeekly = true;
        titleField.text = "";
        timeField.text = "";
        dateField.text = backend.day.currentDate;
        var jsDate = new Date(backend.day.currentDate);
        var isoWeekday = jsDate.getDay() === 0 ? 7 : jsDate.getDay();
        weekdayCombo.currentIndex = isoWeekday - 1;
        open();
    }

    function openForEdit(id) {
        editingEventId = id;
        var data = backend.editor.getEvent(id);
        titleField.text = data.title || "";
        timeField.text = data.time || "";
        open();
    }

    onAccepted: {
        if (editingEventId >= 0) {
            backend.editor.updateEvent(editingEventId, titleField.text, timeField.text);
        } else if (isWeekly) {
            backend.editor.addWeeklyEvent(weekdayCombo.currentIndex + 1, titleField.text, timeField.text);
        } else {
            backend.editor.addOneOffEvent(dateField.text, titleField.text, timeField.text);
        }
    }

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.accent
        border.width: 1
        radius: 6
    }

    header: Label {
        text: dialog.editingEventId >= 0 ? "Editar evento" : "Nuevo evento"
        color: Theme.accent
        font.bold: true
        padding: 14
    }

    footer: RowLayout {
        width: dialog.width
        spacing: 8

        Button {
            Layout.fillWidth: true
            Layout.margins: 8
            onClicked: dialog.reject()
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
            Layout.fillWidth: true
            Layout.margins: 8
            enabled: titleField.text.length > 0
                     && (dialog.editingEventId >= 0 || dialog.isWeekly || dateField.text.length > 0)
            onClicked: dialog.accept()
            contentItem: Text {
                text: "Guardar"
                color: Theme.background
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
            background: Rectangle {
                color: Theme.accent
                radius: 4
                opacity: parent.enabled ? 1.0 : 0.4
            }
        }
    }

    ColumnLayout {
        width: dialog.availableWidth
        spacing: 10

        RowLayout {
            visible: dialog.editingEventId < 0
            Layout.fillWidth: true
            spacing: 6

            Button {
                text: "Semanal"
                Layout.fillWidth: true
                onClicked: dialog.isWeekly = true
                contentItem: Text {
                    text: "Semanal"
                    color: dialog.isWeekly ? Theme.background : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: dialog.isWeekly ? Theme.accent : Theme.surfaceAlt
                    radius: 4
                }
            }
            Button {
                text: "Puntual"
                Layout.fillWidth: true
                onClicked: dialog.isWeekly = false
                contentItem: Text {
                    text: "Puntual"
                    color: !dialog.isWeekly ? Theme.background : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: !dialog.isWeekly ? Theme.accent : Theme.surfaceAlt
                    radius: 4
                }
            }
        }

        ComboBox {
            id: weekdayCombo
            Layout.fillWidth: true
            visible: dialog.editingEventId < 0 && dialog.isWeekly
            model: dialog.weekdayNames
            background: Rectangle { color: Theme.surfaceAlt; radius: 4 }
            contentItem: Text {
                text: weekdayCombo.displayText
                color: Theme.textPrimary
                leftPadding: 8
                verticalAlignment: Text.AlignVCenter
            }
        }

        TextField {
            id: dateField
            Layout.fillWidth: true
            visible: dialog.editingEventId < 0 && !dialog.isWeekly
            placeholderText: "YYYY-MM-DD"
            color: Theme.textPrimary
            placeholderTextColor: Theme.textSecondary
            background: Rectangle { color: Theme.surfaceAlt; radius: 4 }
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            placeholderText: "Título"
            color: Theme.textPrimary
            placeholderTextColor: Theme.textSecondary
            background: Rectangle { color: Theme.surfaceAlt; radius: 4 }
        }

        TextField {
            id: timeField
            Layout.fillWidth: true
            placeholderText: "HH:MM (opcional)"
            color: Theme.textPrimary
            placeholderTextColor: Theme.textSecondary
            background: Rectangle { color: Theme.surfaceAlt; radius: 4 }
        }
    }
}
