import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.0 as QQC1
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    minimumWidth: 500
    minimumHeight: 400
    title: qsTr("openSUSE Image Writer")

    // Background color
    color: "#302020"

    Rectangle {
        id: animDeleg
        color: window.color
        visible: animation.running
        z: 2

        Image {
            id: animImage
            anchors.fill: parent
        }

        ParallelAnimation {
            id: animation
            running: false
            property int duration: 500
            property var targetPos: taskList.mapToGlobal(-window.x, -window.y)

            OpacityAnimator {
                target: animDeleg
                from: 1
                to: 0
                duration: animation.duration*2
                easing.type: Easing.OutSine
            }

            XAnimator {
                target: animDeleg
                from: 0
                to: animation.targetPos.x
                duration: animation.duration
                easing.type: Easing.OutSine
            }

            YAnimator {
                target: animDeleg
                from: 0
                to: animation.targetPos.y
                duration: animation.duration
                easing.type: Easing.OutSine
            }

            NumberAnimation {
                target: animDeleg
                property: "width"
                from: selection.width
                to: taskList.width
                duration: animation.duration
                easing.type: Easing.OutSine
            }

            NumberAnimation {
                target: animDeleg
                property: "height"
                from: selection.height
                to: taskList.height
                duration: animation.duration
                easing.type: Easing.OutSine
            }
        }
    }


    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        Item {
            id: selection
            width: 200
            height: 200
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.topMargin: 10

            ColumnLayout {
                id: sourceLayout
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: go.left

                PlasmaCore.IconItem {
                    width: 64
                    height: 64
                    Layout.maximumHeight: 64
                    Layout.maximumWidth: 64
                    Layout.minimumHeight: 64
                    Layout.minimumWidth: 64
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    source: "start-here-branding"
                }

                Label {
                    id: label1
                    color: "#c2c2c2"
                    text: qsTr("Source Image")
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 11
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                }

                QQC1.ScrollView {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    GridLayout {
                        width: parent.parent.width
                        columns: 2

                        Label {
                            color: "#c2c2c2"
                            text: qsTr("Distribution:")
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                            font.pointSize: 11
                        }

                        ComboBox {
                            id: comb
                            Layout.fillWidth: true
                            model: ImageMetadataStorage
                            textRole: "ChoiceName"
                        }

                        Label {
                            color: "#c2c2c2"
                            text: ImageMetadataStorage.data(ImageMetadataStorage.index(comb.currentIndex, 0, 0), 0x100)
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                            font.pointSize: 11
                        }

                        ComboBox {
                            property var rootIndex: ImageMetadataStorage.index(comb.currentIndex, 0, 0)
                            enabled: count > 0
                            Layout.fillWidth: true

                            model: {
                                var size = ImageMetadataStorage.rowCount(rootIndex);
                                var ret = []
                                for(var row = 0; row < size; ++row)
                                    ret.push(ImageMetadataStorage.data(ImageMetadataStorage.index(row, 0, rootIndex), 0x101));

                                return ret;
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                id: targetLayout
                anchors.rightMargin: 10
                anchors.leftMargin: 10
                anchors.left: go.right
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                PlasmaCore.IconItem {
                    x: 462
                    y: 23
                    Layout.maximumHeight: 64
                    Layout.maximumWidth: 64
                    Layout.minimumWidth: 64
                    Layout.minimumHeight: 64
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    source: "media-optical-recordable"
                }

                Label {
                    id: label
                    color: "#c2c2c2"
                    text: qsTr("Target Disk")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.bold: true
                    font.pointSize: 11
                }

                ComboBox {
                    model: ["DVD in sr0"]

                    id: comboBox
                    x: 434
                    y: 116
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }


                Item {
                    Layout.fillHeight: true
                }
            }

            Item {
                id: go
                width: 100
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter

                // Paint an arrow pointing to the right
                Canvas {
                    width: 80
                    height: 50
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    property int stemThickness: 15
                    property int pointyEndThickness: 25
                    x: 10
                    y: 89

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.fillStyle = "#f2f2f2";
                        ctx.beginPath();
                        ctx.moveTo(0, height/2 - stemThickness/2);
                        ctx.lineTo(width - pointyEndThickness, height/2 - stemThickness/2);
                        ctx.lineTo(width - pointyEndThickness, 0);
                        ctx.lineTo(width, height/2);
                        ctx.lineTo(width - pointyEndThickness, height);
                        ctx.lineTo(width - pointyEndThickness, height/2 + stemThickness/2);
                        ctx.lineTo(0, height/2 + stemThickness/2);
                        ctx.closePath();
                        ctx.stroke();
                        ctx.fill();
                    }
                }

                Button {
                    text: qsTr("Start!")
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.left: parent.left
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    onClicked: {
                        selection.grabToImage(function (image) {
                            animImage.source = image.url;
                            animation.start();
                        });
                    }
                }
            }
        }

        GroupBox {
            id: taskListBox
            width: 200
            height: 200
            Layout.preferredHeight: 120
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 10
            title: qsTr("Task List")

            // Custom label component for the right text color
            label: Label {
                color: "#c2c2c2"
                text: parent.title
            }

            ListView {
                id: taskList
                anchors.fill: parent
                clip: true

                model: ListModel {
                    ListElement { status: "Failed burning openSUSE Leap 42.3 DVD"; progress: 0; color: "#d08080" }
                    ListElement { status: "Finished openSUSE Tumbleweed Kubic"; progress: 100; color: "#90d090" }
                    ListElement { status: "Downloading openSUSE Tumbleweed KDE Live"; progress: 30; color: "#a0a0a0" }
                }

                spacing: 5

                delegate: Rectangle {
                    implicitWidth: taskList.width
                    implicitHeight: 60
                    color: model.color

                    Label {
                        color: "black"
                        text: status

                        anchors {
                            top: parent.top
                            left: parent.left
                            right: percLabel.left
                            margins: 10
                        }
                    }

                    Label {
                        id: percLabel
                        color: "black"
                        text: progress + " %"

                        anchors {
                            top: parent.top
                            right: parent.right
                            margins: 10
                        }
                    }

                    ProgressBar {
                        from: 0
                        to: 100
                        value: progress

                        anchors {
                            bottom: parent.bottom
                            left: parent.left
                            right: parent.right
                            margins: 10
                        }
                    }
                }
            }
        }
    }
}
