import QtQuick 2.0
//import QtQuick.Controls 2.0
import QtQuick.Controls 1.0
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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Layout.fillWidth: true

            Image {
                width: 64
                height: 64
                Layout.maximumHeight: 64
                Layout.maximumWidth: 64
                Layout.minimumHeight: 64
                Layout.minimumWidth: 64
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                source: "qrc:/icons/icons/start-here-branding.svg"
            }

            Label {
                text: qsTr("openSUSE ImageWriter 2")
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                fontSizeMode: Text.Fit
                font.pixelSize: 64
                font.bold: true
                color: dialog.fontColor
            }
        }

        Label {
            text: qsTr("Homepage: <a href=\"%1\">On GitHub</a><br/>" +
                       "An <a href=\"%2\">openSUSE</a> project<br/>" +
                       "Created by Fabian Vogt").arg("https://github.com/openSUSE/imagewriter2").arg("https://www.opensuse.org")
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.fillHeight: true
            onLinkActivated: Qt.openUrlExternally(link)
            Layout.fillWidth: true
            color: dialog.fontColor
            font.pointSize: 12

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                acceptedButtons: Qt.NoButton
            }
        }

        Button {
            anchors {
                right: parent.right
            }

            text: qsTr("Close")
            onClicked: dialog.visible = false
        }
    }
}
