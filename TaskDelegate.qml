import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQml.Models 2.1

Rectangle {
    id: delegate

    implicitWidth: taskList.width
    implicitHeight: column.implicitHeight + 10

    // The background color shows the state
    property var backgroundColors: ["#ffffff", "#a0a0a0", "#90d090", "#d08080"]
    color: backgroundColors[model.State]
    
    ColumnLayout {
        id: column
        anchors {
            fill: parent
            margins: 10
        }
        
        RowLayout {
            id: rowLayout
            Layout.fillWidth: true
            
            Label {
                Layout.fillWidth: true
                color: "black"
                font.pointSize: 11
                text: model.Name
            }
            
            Label {
                id: percLabel
                color: "black"
                text: model.Progress + " %"
            }
        }

        ProgressBar {
            from: 0
            to: 100
            value: model.Progress
            Layout.fillWidth: true
        }

        Label {
            Layout.fillWidth: true
            color: "black"
            font.pointSize: 9
            text: model.Message
        }
        
        DelegateModel {
            id: childTaskModel
            model: taskManager
            rootIndex: taskManager.index(index, 0, 0)
            delegate: Rectangle {
                border.color: "darkgrey"
                border.width: 2
                Layout.leftMargin: column.anchors.margins
                Layout.rightMargin: -column.anchors.margins - border.width
                Layout.fillWidth: true
                implicitHeight: childTaskLayout.implicitHeight
                color: backgroundColors[model.State]

                ColumnLayout {
                    id: childTaskLayout
                    anchors.fill: parent
                    anchors.margins: 5

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            color: "black"
                            font.pointSize: 11
                            text: model.Name
                        }

                        Label {
                            color: "black"
                            text: model.Progress + " %"
                        }
                    }

                    ProgressBar {
                        from: 0
                        to: 100
                        value: model.Progress
                        Layout.fillWidth: true
                    }

                    Label {
                        Layout.fillWidth: true
                        color: "black"
                        font.pointSize: 9
                        text: model.Message
                    }

                    Item {
                        Layout.fillHeight: true
                        implicitHeight: 5
                    }
                }
            }
        }
        
        Repeater {
            Layout.fillWidth: true
            model: childTaskModel
        }

    }
}
