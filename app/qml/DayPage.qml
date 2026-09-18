import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LevelUp

Page {
    id: page

    readonly property int hourHeight: 72
    readonly property real timelineContentHeight: hourHeight * 24

    background: Rectangle { color: Theme.background }

    // "00:00" abajo, "24:00" arriba -- el eje de tiempo que pide el
    // usuario. hhmm es "HH:MM"; se asume ya validado por AddEventDialog.
    function timeToY(hhmm) {
        var parts = hhmm.split(":");
        var minutes = parseInt(parts[0], 10) * 60 + parseInt(parts[1], 10);
        return timelineContentHeight - (minutes / 1440) * timelineContentHeight;
    }

    AddEventDialog {
        id: eventDialog
    }

    Menu {
        id: eventMenu
        property int targetEventId: -1

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.accent
            border.width: 1
            radius: 4
        }
        MenuItem {
            contentItem: Text { text: "Editar"; color: Theme.textPrimary; leftPadding: 8 }
            onTriggered: eventDialog.openForEdit(eventMenu.targetEventId)
        }
        MenuItem {
            contentItem: Text { text: "Borrar"; color: "#e0585f"; leftPadding: 8 }
            onTriggered: backend.editor.deleteEvent(eventMenu.targetEventId)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                contentItem: Text { text: "<"; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter }
                background: Rectangle { color: "transparent" }
                onClicked: backend.day.prevDay()
            }
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                color: Theme.textPrimary
                text: backend.day.currentDateLabel
            }
            ToolButton {
                contentItem: Text { text: ">"; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter }
                background: Rectangle { color: "transparent" }
                onClicked: backend.day.nextDay()
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            onClicked: backend.day.goToToday()
            contentItem: Text { text: "Hoy"; color: Theme.accent; horizontalAlignment: Text.AlignHCenter }
            background: Rectangle {
                color: "transparent"
                border.color: Theme.accent
                border.width: 1
                radius: 4
            }
        }

        // Eventos sin hora fija -- fuera de la timeline temporal para no
        // complicar la matematica de posicion (ver timeToY).
        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: backend.day.events
                delegate: Rectangle {
                    visible: model.time === ""
                    radius: 10
                    height: 28
                    width: chipLabel.implicitWidth + 16
                    color: model.done ? Theme.surfaceAlt : Theme.surface
                    border.color: Theme.accent
                    border.width: 1

                    Label {
                        id: chipLabel
                        anchors.centerIn: parent
                        text: model.title
                        color: Theme.textPrimary
                        font.strikeout: model.done
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: backend.day.toggleCompletion(model.eventId)
                        onPressAndHold: {
                            eventMenu.targetEventId = model.eventId;
                            eventMenu.popup();
                        }
                    }
                }
            }
        }

        Flickable {
            id: timelineFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: page.timelineContentHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            // Centrar en la hora actual (o en las 08:00 si no es hoy) una
            // sola vez, cuando el Flickable ya tiene una altura real --
            // en Component.onCompleted, dentro de un ColumnLayout, height
            // todavia vale 0 (el layout no ha corrido su primer pase).
            property bool initialScrollDone: false
            onHeightChanged: {
                if (initialScrollDone || height <= 0) {
                    return;
                }
                initialScrollDone = true;
                var now = new Date();
                var isToday = backend.day.currentDate === Qt.formatDate(now, "yyyy-MM-dd");
                var targetMinutes = isToday ? (now.getHours() * 60 + now.getMinutes()) : (8 * 60);
                var targetY = page.timelineContentHeight - (targetMinutes / 1440) * page.timelineContentHeight;
                contentY = Math.max(0, Math.min(page.timelineContentHeight - height, targetY - height / 2));
            }

            Item {
                width: timelineFlick.width
                height: page.timelineContentHeight

                Repeater {
                    model: 25
                    Label {
                        readonly property int hour: index
                        x: 0
                        width: 48
                        y: page.timelineContentHeight - hour * page.hourHeight - font.pixelSize / 2
                        horizontalAlignment: Text.AlignRight
                        color: Theme.textSecondary
                        text: (hour < 10 ? "0" + hour : hour) + ":00"
                    }
                }

                Rectangle {
                    id: spine
                    x: 56
                    y: 0
                    width: 2
                    height: parent.height
                    color: Theme.accent
                    opacity: 0.5
                }

                Repeater {
                    model: backend.day.events
                    delegate: Item {
                        id: eventItem
                        visible: model.time !== ""
                        x: 0
                        y: 0
                        width: parent.width
                        height: parent.height

                        readonly property bool hasDuration: visible && model.endTime !== ""
                        readonly property real yStart: visible ? page.timeToY(model.time) : 0
                        readonly property real yEnd: hasDuration ? page.timeToY(model.endTime) : yStart
                        // Card mas alta cuanto mas dure el evento -- con un
                        // minimo legible para eventos cortos o sin fin.
                        readonly property real cardTop: hasDuration ? Math.min(yStart, yEnd) - 12 : yStart - 22
                        readonly property real cardHeight: hasDuration
                            ? Math.max(Math.abs(yStart - yEnd), 36) + 12
                            : 44

                        // Barra de duracion sobre la spine -- solo si hay
                        // hora de fin, para visualizar de un vistazo cuanto
                        // dura el evento (no solo cuando empieza).
                        Rectangle {
                            visible: eventItem.hasDuration
                            x: spine.x - 1
                            y: Math.min(eventItem.yStart, eventItem.yEnd)
                            width: 4
                            height: Math.abs(eventItem.yStart - eventItem.yEnd)
                            radius: 2
                            color: Theme.accent
                            opacity: model.done ? 0.25 : 0.5
                        }

                        Rectangle {
                            x: spine.x - 5
                            y: eventItem.yStart - 5
                            width: 10
                            height: 10
                            radius: 5
                            color: model.done ? Theme.accent : Theme.background
                            border.color: Theme.accent
                            border.width: 2
                        }

                        Rectangle {
                            x: spine.x + 20
                            y: eventItem.cardTop
                            width: timelineFlick.width - spine.x - 40
                            height: eventItem.cardHeight
                            radius: 8
                            color: Theme.surface
                            border.width: 1
                            border.color: model.done ? Theme.surfaceAlt : Theme.accent
                            opacity: model.done ? 0.6 : 1.0

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0
                                    Label {
                                        Layout.fillWidth: true
                                        text: model.title
                                        color: Theme.textPrimary
                                        font.strikeout: model.done
                                        elide: Text.ElideRight
                                    }
                                    Label {
                                        text: eventItem.hasDuration ? (model.time + " - " + model.endTime) : model.time
                                        color: Theme.textSecondary
                                        font.pixelSize: 11
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: backend.day.toggleCompletion(model.eventId)
                                onPressAndHold: {
                                    eventMenu.targetEventId = model.eventId;
                                    eventMenu.popup();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
