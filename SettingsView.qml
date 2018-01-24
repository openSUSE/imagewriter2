import QtQuick 2.0
//import QtQuick.Controls 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.3

import org.opensuse.imgwriter 1.0

GridLayout {
    columns: 2

    // This property needs to be updated occasionally
    property string cacheSize: CacheHelper.getCacheSize()

    onVisibleChanged: cacheSize = CacheHelper.getCacheSize()

    Item {
        Layout.columnSpan: 2
        Layout.fillHeight: true
    }

    CheckBox {
        id: downloadOnlyCheckbox
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    }

    Label {
        text: qsTr("Only download images (this session only)")
        color: window.fontColor

        MouseArea {
            anchors.fill: parent
            onClicked: downloadOnlyCheckbox.checked ^= 1
        }
    }



    Label {
        text: qsTr("Cache:")
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        color: window.fontColor
    }

    Label {
        id: cacheSizeLabel
        text: parent.cacheSize
        color: window.fontColor
    }



    Label {
    }

    Button {
        text: qsTr("Clear cache")

        onClicked: {
            CacheHelper.clearCache(ims.serviceName)
            cacheSize = CacheHelper.getCacheSize()
        }
    }



    Item {
        Layout.columnSpan: 2
        Layout.fillHeight: true
    }
}
