import QtQuick 2.0
import QtPositioning 5.2
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1

Page {
    id: page

    property string stationId

    allowedOrientations: Orientation.All

    Station {
        id: station
        stationId: page.stationId
    }

    Address {
        id: stationAddress
        address: station.address
    }

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: page.width

            PageHeader {
                title: station.brand
            }

            Label {
                text: stationAddress.text.replace(', ', "\n")
            }

            Label {
                text: station.isOpen ? qsTr("Open") : qsTr("Closed")
            }

            SectionHeader {
                text: qsTr("Prices")
                visible: station.isOpen
            }

            Repeater {
                width: page.width

                model: ListModel {
                    ListElement {
                      name: qsTr("Super E5")
                      fuelId: FuelPriceProvider.SuperE5
                    }
                    ListElement {
                      name: qsTr("Super E10")
                      fuelId: FuelPriceProvider.SuperE10
                    }
                    ListElement {
                      name: qsTr("Diesel")
                      fuelId: FuelPriceProvider.Diesel
                    }
                }

                delegate: DetailItem {
                    id: priceItem
                    property real price: Number.NaN
                    property var formatedPrice: formatPrice(price)

                    width: parent.width
                    height: Theme.itemSizeSmall

                    label: name
                    value: formatedPrice[0]
                    visible: !isNaN(price)

                    Component.onCompleted: {
                        var fuel = fuelId
                        station.detailsFetched.connect(function() {
                            priceItem.price = station.priceFor(fuel)
                        })
                    }
                }
            }
        }

        ViewPlaceholder {
            enabled: station.status === Station.Error
            text: station.errorString
        }

        BusyLabel {
            running: station.status === Station.Loading
        }

        PullDownMenu {
            flickable: content

            MenuItem {
                text: qsTr("Update")
                onClicked: station.update()
            }
        }

        VerticalScrollDecorator { flickable: content }
    }

    Component.onCompleted: {
        station.provider = provider
        station.fetchDetails()
    }
}
