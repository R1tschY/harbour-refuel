import QtQuick 2.0
import QtPositioning 5.2
import QtLocation 5.3
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1

Dialog {
    id: page

    allowedOrientations: Orientation.All

//    PositionSource {
//        id: position
//        active: gpsActive
//    }

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: page.width

            DialogHeader {
                width: parent.width
                acceptText: qsTr("Search")
                cancelText: qsTr("Cancel")
            }

            Plugin {
                    id: mapPlugin
                    name: "osm" // "mapboxgl", "esri", ...
                    // specify plugin parameters if necessary
                    // PluginParameter {
                    //     name:
                    //     value:
                    // }
                }

            Map {
                id: map
                plugin: mapPlugin
                width: parent.width
                height: Theme.paddingLarge * 20
                zoomLevel: 14

                center {
                    latitude: -27.5796
                    longitude: 153.1003
                }

                // Enable pinch gestures to zoom in and out
                gesture.flickDeceleration: 3000
                gesture.enabled: true

            }



            ValueButton {
                id: locationControl

                property real lat: -1
                property real lng: -1
                property bool currentPos: true

                label: qsTr("Near")
                value: qsTr("Current Position")

                BusyIndicator {
                    running: locationControl.lat === -1
                    size: BusyIndicatorSize.Small
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                }

                onClicked: pageStack.push(locationChooserDialogComponent)
            }

            ComboBox {
                id: fuelTypeChooser

                width: parent.width

                label: qsTr("Fuel type")
                menu: ContextMenu {
                    Repeater {
                        model: [
                            {
                                name: qsTr("Super E5"),
                                ty: Request.SuperE5
                            },
                            {
                                name: qsTr("Super E10"),
                                ty: Request.SuperE10
                            },
                            {
                                name: qsTr("Diesel"),
                                ty: Request.Diesel
                            }
                        ]

                        MenuItem {
                            readonly property int fuelType: modelData.ty

                            text: modelData.name
                        }
                    }
                }
            }

            ComboBox {
                id: radiusChooser

                width: parent.width

                label: qsTr("Search radius")
                currentIndex: 2

                menu: ContextMenu {
                    Repeater {
                        model: [1, 2, 5, 10, 15, 20, 25]
                        MenuItem {
                            readonly property string radius: modelData

                            text: modelData + " km"
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {
            flickable: content
        }
    }

    onAccepted: pageStack.push(
                    Qt.resolvedUrl("StationListPage.qml"),
                    { fuelType: fuelTypeChooser.currentItem.fuelType,
                        radius: radiusChooser.currentItem.radius,
                        latitude: 52.521,
                        longitude: 13.438 })
}
