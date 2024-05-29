import QtQuick 2.0
import QtPositioning 5.2
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1
import "../components"

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

            SectionHeader {
                text: qsTr("Prices")
                visible: station.isOpen
            }

            Column {
                width: page.width
                spacing: Theme.paddingMedium

                Repeater {
                    width: page.width
                    visible: station.isOpen

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

                    delegate: Item {
                        id: priceItem
                        property real price: Number.NaN
                        property var formatedPrice: formatPrice(price)

                        width: parent.width
                        height: priceLabel.height + Theme.paddingSmall

                        PriceDisplay {
                            id: priceLabel

                            anchors {
                                right: parent.horizontalCenter
                                rightMargin: Theme.paddingSmall
                            }

                            color: Theme.highlightColor
                            placeholderColor: Theme.secondaryHighlightColor
                            mainPrice: formatedPrice[0]
                            decimalPrice: formatedPrice[1]
                        }

                        Label {
                            id: fuelLabel

                            anchors {
                                left: parent.horizontalCenter
                                leftMargin: Theme.paddingSmall
                                verticalCenter: priceLabel.verticalCenter
                            }

                            text: name
                            color: Theme.highlightColor
                        }

                        visible: !isNaN(price)

                        Component.onCompleted: {
                            var fuel = fuelId
                            station.updated.connect(function() {
                                priceItem.price = station.priceFor(fuel)
                            })
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Opening Hours")
            }

            Label {
                text: station.isOpen ? qsTr("Open") : qsTr("Closed")
                color: Theme.highlightColor
                horizontalAlignment: Text.AlignHCenter

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
            }

            DetailItem {
                label: qsTr("Daily")
                value: "00:00 - 24:00"
                visible: station.wholeDay

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
            }

            Repeater {
                width: page.width

                model: station.openingTimes

                delegate: DetailItem {
                    label: modelData.weekDays !== 0 ? formatWeekDays(modelData.weekDays) : modelData.weekDaysText
                    value: formatTime(modelData.start) + " - " + formatTime(modelData.end)

                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                    }
                }
            }

            Repeater {
                width: page.width

                model: station.openingTimesOverrides

                delegate: Label {
                    text: modelData
                     color: Theme.highlightColor

                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                    }
                }
            }

            SectionHeader {
                text: qsTr("Address")
            }

            BackgroundItem {
                id: addressItem

                Label {
                    text: stationAddress.text.replace(', ', "\n")
                    color: addressItem.highlighted
                           ? Theme.highlightColor : Theme.primaryColor

                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                    }
                }

                onClicked: {
                    var uri = geoUri(station.coordinate)
                    console.log("Open geo location " + uri)
                    if (!Qt.openUrlExternally(uri)) {
                        feedback.info(qsTr("No application installed to open geo location"))
                    }
                }
            }

            FooterText {
                text: provider.copyright
            }
        }

        ViewPlaceholder {
            enabled: station.detailsStatus === Station.Error
            text: station.detailsErrorString
        }

        BusyLabel {
            running: station.detailsStatus === Station.Loading
        }

        PullDownMenu {
            flickable: content
            busy: station.status === Station.Loading

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

    function formatWeekDays(weekDays) {
        if (weekDays === 0xFF) {
            return qsTr("Daily")
        }

        var res = []
        if ((weekDays & 0x1f) === 0x1f) {
            res.push(qsTr("Monday") + " - " + qsTr("Friday"))
            weekDays &= ~0x1f;
        }
        for (var i = 0; i < 8; i++) {
            if ((weekDays & (1 << i)) !== 0) {
                switch (i) {
                case 0: res.push(qsTr("Monday")); break;
                case 1: res.push(qsTr("Tuesday")); break;
                case 2: res.push(qsTr("Wednesday")); break;
                case 3: res.push(qsTr("Thursday")); break;
                case 4: res.push(qsTr("Friday")); break;
                case 5: res.push(qsTr("Saturday")); break;
                case 6: res.push(qsTr("Sunday")); break;
                case 7: res.push(qsTr("Public Holiday")); break;
                }
            }
        }
        return res.join(", ")
    }

    function formatTime(time) {
        var hours = String(time.getHours())
        if (hours.length == 1) {
            hours = "0" + hours
        }
        var min = String(time.getMinutes())
        if (min.length == 1) {
            min = "0" + min
        }
        return hours + ":" + min
    }
}
