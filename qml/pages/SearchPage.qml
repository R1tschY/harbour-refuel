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

            ValueButton {
                id: locationChooser

                width: parent.width

                property variant coordinate: QtPositioning.coordinate()
                property string address
                property bool currentPos: false

                label: qsTr("Near")
                value: currentPos
                       ? qsTr("Current Position")
                       : address

                BusyIndicator {
                    running: locationChooser.currentPos && coordinate.valid // TODO
                    size: BusyIndicatorSize.Small
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                }

                onClicked: {
                    var dialog = pageStack.push(
                               Qt.resolvedUrl("ChooseLocationDialog.qml"))
                    dialog.selected.connect(function(location) {
                        locationChooser.coordinate = location.coordinate
                        locationChooser.address = location.address.text
                        locationChooser.currentPos = false
                    })
                }
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
                                fuel: FuelPriceProvider.SuperE5
                            },
                            {
                                name: qsTr("Super E10"),
                                fuel: FuelPriceProvider.SuperE10
                            },
                            {
                                name: qsTr("Diesel"),
                                fuel: FuelPriceProvider.Diesel
                            }
                        ]

                        MenuItem {
                            readonly property int fuel: modelData.fuel

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

    onAccepted:
        pageStack.push(
                    Qt.resolvedUrl("StationListPage.qml"),
                    { fuel: fuelTypeChooser.currentItem.fuel,
                        radius: radiusChooser.currentItem.radius,
                        coordinate: locationChooser.coordinate })
}
