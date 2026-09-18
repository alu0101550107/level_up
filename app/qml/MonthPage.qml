import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LevelUp

Page {
    id: page

    readonly property var weekdayLabels: ["L", "M", "X", "J", "V", "S", "D"]

    background: Rectangle { color: Theme.background }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                contentItem: Text { text: "<"; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter }
                background: Rectangle { color: "transparent" }
                onClicked: backend.month.prevMonth()
            }
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                color: Theme.textPrimary
                text: backend.month.monthLabel
            }
            ToolButton {
                contentItem: Text { text: ">"; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter }
                background: Rectangle { color: "transparent" }
                onClicked: backend.month.nextMonth()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Repeater {
                model: page.weekdayLabels
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    color: Theme.textSecondary
                    text: modelData
                }
            }
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            interactive: false
            cellWidth: width / 7
            cellHeight: cellWidth
            model: backend.month.grid

            delegate: Rectangle {
                width: grid.cellWidth
                height: grid.cellHeight
                color: "transparent"
                radius: 6
                border.width: model.isToday ? 1 : 0
                border.color: Theme.accent

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: model.dayNumber
                        color: Theme.textPrimary
                        opacity: model.isCurrentMonth ? 1.0 : 0.3
                    }
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: model.hasEvents
                        width: 6
                        height: 6
                        radius: 3
                        color: model.allDone ? Theme.accent : Theme.textSecondary
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: backend.month.activateDate(model.date)
                }
            }
        }
    }
}
