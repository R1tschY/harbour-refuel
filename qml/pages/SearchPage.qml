import QtQuick 2.0
import QtPositioning 5.2
import Sailfish.Silica 1.0

import de.richardliebscher.tanken 0.1

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
                acceptText: qsTr("Search")
                cancelText: qsTr("Cancel")
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
