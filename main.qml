import QtQuick 2.0
import QtQuick.Controls 2.0
//import QtQuick.Controls 1.0
import QtQuick.Controls 1.0 as QQC1
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1

import org.opensuse.imgwriter 1.0

ApplicationWindow {
    property string fontColor: "#e0e0e0"

    id: window
    visible: true
    width: 730
    height: 480
    minimumWidth: 500
    minimumHeight: 400
    title: qsTr("openSUSE Image Writer")

    // Background color
    color: "#302020"

    Component.onCompleted: {
        var mdt = taskManager.createMetadataDownloadTask("opensuse");
        mdt.finished.connect(function (url) { ims.readFromXMLFile(url); });
        mdt.start();
    }

    Item {
        anchors.fill: parent

        ImageMetadataStorage {
            id: ims
            serviceName: "opensuse"
        }

        TaskManager {
            id: taskManager
        }

        ColumnLayout {
            anchors.margins: 5
            anchors.fill: parent

            GridLayout {
                id: selection
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.topMargin: 10

                signal selectedIndexChanged()

                /* Returns QModelIndex into ims to get the data about the chosen image. */
                function getCurrentSelectedIndex()
                {
                    /* Iterate all comboboxes, the last one indicates which image got picked. */
                    for(var i = comboboxRepeater.count; --i >= 0;)
                    {
                        var combobox = comboboxRepeater.itemAt(i);
                        if(!combobox || !combobox.visible)
                            continue;

                        return ims.index(combobox.currentIndex, 0, combobox.rootIndex);
                    }
                }

                ColumnLayout {
                    id: sourceLayout
                    Layout.fillHeight: true
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
                        color: window.fontColor
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

                            /* This repeater creates the labels. They get their information on what to display from their
                               "parent" ComboBox, so those have to be initialized first. To make sure that that happens,
                               model: is bound to comboboxRepeater.model. */
                            Repeater {
                                id: labelRepeater
                                model: comboboxRepeater.model
                                delegate: Label {
                                    property var parentCombobox: index === 0 ? null : comboboxRepeater.itemAt(index - 1)
                                    property var rootIndex: parentCombobox ? ims.index(parentCombobox.currentIndex, 0, parentCombobox.rootIndex) : 0
                                    visible: !parentCombobox || (parentCombobox.visible && ims.hasChildren(rootIndex))

                                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                    horizontalAlignment: Text.AlignRight

                                    color: window.fontColor
                                    font.pointSize: 11

                                    Layout.row: index
                                    Layout.column: 0

                                    text: ims.data(rootIndex, ImageMetadataStorage.DecisionNameRole) + ":"
                                }
                            }

                            Repeater {
                                id: comboboxRepeater
                                model: ims.maxDepth
                                delegate: ComboBox {
                                    property var parentCombobox: index === 0 ? null : comboboxRepeater.itemAt(index - 1)
                                    property var rootIndex: parentCombobox ? ims.index(parentCombobox.currentIndex, 0, parentCombobox.rootIndex) : 0
                                    visible: !parentCombobox || (parentCombobox.visible && ims.hasChildren(rootIndex))

                                    Layout.fillWidth: true

                                    enabled: model.length > 1

                                    Layout.row: index
                                    Layout.column: 1

                                    // Trigger recalculation of the selected image
                                    onCurrentIndexChanged: selection.selectedIndexChanged();
                                    onRootIndexChanged: selection.selectedIndexChanged();

                                    // Generate the list of possible options based on the rootIndex
                                    model: {
                                        var size = ims.rowCount(rootIndex);
                                        var ret = []
                                        for(var row = 0; row < size; ++row)
                                            ret.push(ims.data(ims.index(row, 0, rootIndex), ImageMetadataStorage.OptionNameRole));

                                        return ret;
                                    }

                                    // If the list changed, we need to preselect an option. Use the information from the model
                                    onModelChanged: {
                                        currentIndex = ims.data(rootIndex, ImageMetadataStorage.DecisionPreselectedOptionRole) || 0
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }

                Item {
                    id: go
                    Layout.minimumWidth: startButton.width
                    Layout.minimumHeight: startButton.height
                    Layout.preferredHeight: startButton.height
                    Layout.fillWidth: false
                    Layout.fillHeight: true

                    // This function checks that there's nothing wrong with the options
                    // the user selected.
                    function refreshValidationState()
                    {
                        var valid = true;
                        var errormsgs = [];

                        // Verify that a valid image is selected
                        var index = selection.getCurrentSelectedIndex();
                        var imageSize = index ? ims.data(index, ImageMetadataStorage.ImageSizeRole) : undefined;
                        if(!imageSize)
                        {
                            valid = false;
                            errormsgs.push(qsTr("No valid image selected"));
                        }

                        // Verify that a valid drive is selected
                        var driveSize = targetSelection.driveSize
                        if(!driveSize)
                        {
                            valid = false;
                            errormsgs.push(qsTr("No valid drive selected"));
                        }

                        // If valid image and drive are selected, verify the size
                        if(valid && Size64Comparator.compare(imageSize, driveSize) < 0)
                        {
                            valid = false;
                            errormsgs.push(qsTr("The selected drive is too small for the image"));
                        }

                        if(valid)
                        {
                            errormsgs.push(qsTr("Ready to write!"));
                        }

                        startButton.enabled = valid;
                        validationList.model = errormsgs;
                    }

                    Connections {
                        target: selection
                        onSelectedIndexChanged: go.refreshValidationState();
                    }

                    Connections {
                        target: targetSelection
                        onDriveSizeChanged: go.refreshValidationState();
                    }

                    // Paint an arrow pointing to the right
                    Canvas {
                        id: arrow
                        width: 80
                        height: 50
                        anchors.horizontalCenter: parent.horizontalCenter
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        property int stemThickness: 15
                        property int pointyEndThickness: 25
                        x: 10
                        y: 89

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.fillStyle = window.fontColor;
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

                    MessageDialog {
                        id: writeConfirmationDialog
                        title: qsTr("Overwrite data?")
                        text: qsTr("By continuing here, you will LOSE ALL DATA on the Device\n%1!\nAre you sure?").arg(targetSelection.driveName)
                        icon: StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.Abort

                        onYes: {
                            var data = ims.data(selection.getCurrentSelectedIndex(), ImageMetadataStorage.ImageDataRole);
                            var driveName = targetSelection.driveName;
                            var driveType = targetSelection.driveType;
                            var drivePath = targetSelection.drivePath;

                            var targetFD = -1;
                            var task;
                            if(driveType != 1) // If not DVD
                            {
                                targetFD = targetSelection.model.openDeviceHandle(targetSelection.currentIndex);
                                if(targetFD < 0)
                                    return;

                                task = taskManager.createImageDownloadWriterTaskUSB(data, ims.serviceName, driveName, targetFD);
                            }
                            else
                            {
                                task = taskManager.createImageDownloadWriterTaskDVD(data, ims.serviceName, driveName, drivePath);
                            }

                            task.start();
                        }
                    }

                    Button {
                        id: startButton
                        enabled: false
                        text: qsTr("Start!")
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        Layout.maximumWidth: 100

                        onClicked: writeConfirmationDialog.visible = true;
                    }
                }

                ColumnLayout {
                    id: targetLayout
                    anchors.rightMargin: 10
                    anchors.leftMargin: 10
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Image {
                        Layout.maximumHeight: 64
                        Layout.maximumWidth: 64
                        Layout.minimumWidth: 64
                        Layout.minimumHeight: 64
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                        // Follows order of RemovableDevicesModel.DeviceType enum
                        property var icons: ["media-optical-recordable.svg",
                                             "media-optical-recordable.svg",
                                             "drive-removable-media-usb-pendrive.svg",
                                             "drive-removable-media-usb-pendrive.svg"]
                        source: "qrc:/icons/icons/" + icons[targetSelection.driveType]
                    }

                    Label {
                        color: window.fontColor
                        text: qsTr("Target Disk")
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        font.bold: true
                        font.pointSize: 11
                    }

                    ComboBox {
                        /* Add an artifical dependency on count to refresh these values when a single entry vanishes */
                        property int driveType: { count; model.data(model.index(currentIndex, 0, 0), 0x102) || 0 }
                        property var driveSize: { count; model.data(model.index(currentIndex, 0, 0), 0x101) || 0 }
                        property var drivePath: { count; model.data(model.index(currentIndex, 0, 0), 0x100) }
                        property var driveName: { count; model.data(model.index(currentIndex, 0, 0), 0) }

                        id: targetSelection

                        model: RemovableDevicesModel {}
                        textRole: "Name"
                        currentIndex: -1
                        enabled: count > 0

                        onCountChanged: {
                            if(currentIndex < 0
                                    || currentIndex >= model.rowCount(0))
                                currentIndex = 0;
                        }

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }

                    Item {
                        Layout.fillHeight: false
                    }

                    ListView {
                        id: validationList
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        model: ["Error 1", "Error 2", "Error 3"]
                        delegate: Label {
                            width: validationList.width
                            text: modelData
                            font.bold: true
                            font.pointSize: 12
                            color: startButton.enabled ? "#9d9" : "#d88"
                            wrapMode: Text.WordWrap
                        }
                    }
                }

            }

            Rectangle {
                id: taskListBox
                width: 200
                height: 200
                border.color: "white"
                border.width: 1
                Layout.preferredHeight: 120
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "transparent"

                // Custom label component for the right text color
                Label {
                    x: 0
                    y: -12
                    color: window.fontColor
                    text: qsTr("Task List")

                    anchors.bottom: parent.top
                    anchors.left: parent.left
                }

                ListView {
                    id: taskList
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true

                    model: taskManager

                    spacing: 5

                    // Scroll to the bottom if a task got added
                    Connections {
                        target: taskManager
                        onTaskAdded: taskList.currentIndex = taskList.count - 1
                    }

                    delegate: TaskDelegate {
                    }
                }
            }
        }

        states: [
            State {
                name: "vertical"
                when: height > width
                PropertyChanges {
                    target: selection
                    columns: 1
                    rows: 3
                }

                PropertyChanges {
                    target: sourceLayout
                    Layout.row: 1
                    Layout.column: 1
                }

                PropertyChanges {
                    target: go
                    Layout.row: 3
                    Layout.column: 1
                }

                PropertyChanges {
                    target: targetLayout
                    Layout.row: 2
                    Layout.column: 1
                }

                PropertyChanges {
                    target: arrow
                    visible: false
                }

                PropertyChanges {
                    target: go
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                }
            },

            State {
                name: "horizontal"
                when: width > height
                PropertyChanges {
                    target: selection
                    columns: 3
                    rows: 1
                }

                PropertyChanges {
                    target: sourceLayout
                    Layout.row: 1
                    Layout.column: 1
                }

                PropertyChanges {
                    target: go
                    Layout.row: 1
                    Layout.column: 2
                }

                PropertyChanges {
                    target: targetLayout
                    Layout.row: 1
                    Layout.column: 3
                }

                PropertyChanges {
                    target: arrow
                    visible: true
                }

                PropertyChanges {
                    target: go
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                }
            }]
    }
}
