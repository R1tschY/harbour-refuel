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

    canAccept: locationChooser.validPosition

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
                readonly property bool validPosition: !isNaN(coordinate.latitude)


                label: qsTr("Near")
                value: currentPos
                       ? qsTr("Current Position")
                       : address

                labelColor: highlighted
                            ? Theme.highlightColor
                            : validPosition ? Theme.primaryColor : Theme.errorColor

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
                            FuelPriceProvider.SuperE5,
                            FuelPriceProvider.SuperE10,
                            FuelPriceProvider.Diesel
                        ]

                        MenuItem {
                            readonly property int fuel: modelData

                            text: formatFuel(fuel)
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

    onAccepted: {
        var fuel = fuelTypeChooser.currentItem.fuel
        var radius = radiusChooser.currentItem.radius
        var coordinate = locationChooser.coordinate
        var name = locationChooser.value
        var currentPos = locationChooser.currentPos

        pageStack.push(
                    Qt.resolvedUrl("StationListPage.qml"),
                    { fuel: fuel, radius: radius, coordinate: coordinate })

        lastSearchesModel.add(
                    Date.now(),
                    "tankerkoenig",
                    locationChooser.value,
                    currentPos ? QtPositioning.coordinate() : coordinate,
                    fuel,
                    radius)
    }
}
