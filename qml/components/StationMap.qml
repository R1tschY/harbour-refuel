import QtQuick 2.0
import QtLocation 5.4

Map {
    property Item flickable
    property bool mapReady: false

    gesture {
        enabled: true
    }

    MouseArea {
        anchors.fill: parent

        onPressed: {
            if (flickable) {
                flickable.interactive = false
            }
            page.enabled = false
        }
        onReleased: {
            if (flickable) {
                flickable.interactive = true
            }
            page.enabled = true
        }
    }

    onSupportedMapTypesChanged: mapReady = true
}
