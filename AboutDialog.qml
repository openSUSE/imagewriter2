import QtQuick 2.0
//import QtQuick.Controls 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0

Window {
    property string fontColor: "#e0e0e0"

    id: dialog
    width: 320
    height: 220
    minimumHeight: height
    maximumHeight: height
    minimumWidth: width
    maximumWidth: width
    title: qsTr("About openSUSE Image Writer")

    // Background color
    color: "#302020"

    TabView {
        id: tabView
        anchors.fill: parent
        anchors.margins: 5

        style: TabViewStyle {
            frameOverlap: 1
            padding.bottom: 10
            tab: Rectangle {
                color: styleData.selected ? window.fontColor : dialog.color
                border.color: window.fontColor
                implicitWidth: (dialog.width - tabView.anchors.margins * 2 + (tabView.count - 1) * frameOverlap) / tabView.count
                implicitHeight: 25

                Label {
                    id: label
                    anchors.centerIn: parent
                    text: styleData.title
                    font.bold: true
                    color: styleData.selected ? dialog.color : window.fontColor
                }
            }
            frame: Rectangle {
                color: dialog.color
                border.color: window.fontColor
                border.width: 1
                anchors.margins: 10
            }
        }

        Tab {
            title: qsTr("About")
            Item {
                AboutView {
                    anchors.fill: parent
                    anchors.margins: 5
                }
            }
        }
        Tab {
            title: qsTr("Settings")
            Item {
                SettingsView {
                    anchors.fill: parent
                    anchors.margins: 5
                }
            }
        }
    }
}
