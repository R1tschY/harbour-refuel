import QtQuick 2.0
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1


Page {
    id: page

    property variant coordinate
    property real radius
    property int fuel

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        model: StationListModel {
            id: listModel
        }

        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Stations")
        }

        delegate: ListItem {
            id: delegate

            contentHeight: Theme.itemSizeMedium

            property var priceParts: formatPrice(price)
            property string mainPrice: priceParts[0]
            property string decimalPrice: priceParts[1]

            property var primaryColor: delegate.highlighted
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
            property var secondaryColor: delegate.highlighted
                                         ? Theme.secondaryHighlightColor
                                         : Theme.secondaryColor

            Label {
                id: priceLabel

                text: mainPrice

                x: Theme.horizontalPageMargin

                font.pixelSize: Theme.fontSizeExtraLarge
                color: primaryColor
            }

            Label {
                id: decimalPriceLabel

                text: decimalPrice

                y: Theme.paddingSmall
                anchors.left: priceLabel.right

                font.pixelSize: Theme.fontSizeSmall
                color: secondaryColor
            }

            Label {
                id: distanceLabel

                text: distance.toFixed(1) + " km"

                anchors {
                    bottom: parent.bottom
                    right: decimalPriceLabel.right
                    rightMargin: Theme.paddingSmall
                }

                font.pixelSize: Theme.fontSizeSmall
                color: secondaryColor
            }

            Label {
                id: nameLabel

                text: brand || name

                anchors {
                    verticalCenter: priceLabel.verticalCenter
                    left: decimalPriceLabel.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                }

                font.pixelSize: Theme.fontSizeLarge
                color: primaryColor
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: addressLabel

                text: address

                anchors {
                    left: decimalPriceLabel.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                    bottom: parent.bottom
                }

                color: secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }

            onClicked: pageStack.push(
                           Qt.resolvedUrl("StationDetailsPage.qml"),
                           {
                               stationId: stationId,
                               stationName: name,
                               stationBrand: brand,
                               stationAddress: address,
                               stationIsOpen: isOpen
                           })

        }

        ViewPlaceholder {
            enabled: listModel.status === StationListModel.Error
            text: listModel.errorString
        }

        ViewPlaceholder {
            enabled: listView.count === 0 && listModel.status === StationListModel.Ready
            text: qsTr("No station found")
        }

        BusyLabel {
            running: listModel.status === StationListModel.Loading
        }

        VerticalScrollDecorator { flickable: listView }
    }

    Component.onCompleted: listModel.search(coordinate, radius, fuel)
}
