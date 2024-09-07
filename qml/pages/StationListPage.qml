import QtQuick 2.0
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1
import "../components"


Page {
    id: page

    property variant coordinate
    property real radius
    property string fuelId

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        model: StationListModel {
            id: listModel
        }

        Component.onCompleted: listModel.provider = provider

        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Stations")
        }

        footer: FooterText {
            text: provider.copyright
            visible: listView.count > 0
                     && listModel.status === StationListModel.Ready
        }

        delegate: ListItem {
            id: delegate

            contentHeight: Theme.itemSizeMedium

            property var priceParts: formatPrice(price)

            property var primaryColor: delegate.highlighted
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
            property var secondaryColor: delegate.highlighted
                                         ? Theme.secondaryHighlightColor
                                         : Theme.secondaryColor

            PriceDisplay {
                id: priceLabel

                y: Theme.paddingMedium * 1.1
                x: Theme.horizontalPageMargin * 0.5

                color: delegate.primaryColor
                placeholderColor: delegate.secondaryColor

                mainPrice: priceParts[0]
                decimalPrice: priceParts[1]
            }

            Label {
                id: distanceLabel

                text: distance.toFixed(1) + " km"

                anchors {
                    bottom: parent.bottom
                    bottomMargin: Theme.paddingSmall
                    right: priceLabel.right
                    rightMargin: Theme.paddingSmall
                }

                font.pixelSize: Theme.fontSizeSmall
                color: secondaryColor
            }

            Label {
                id: nameLabel

                text: brand || name

                anchors {
                    baseline: priceLabel.baseline
                    baselineOffset: -Theme.paddingSmall
                    left: priceLabel.right
                    leftMargin: Theme.paddingMedium
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }

                font.pixelSize: Theme.fontSizeLarge
                color: primaryColor
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: addressLabel

                text: address

                anchors {
                    left: priceLabel.right
                    leftMargin: Theme.paddingMedium
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    bottom: parent.bottom
                    bottomMargin: Theme.paddingSmall
                }

                color: secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }

            onClicked: pageStack.push(
                           Qt.resolvedUrl("StationDetailsPage.qml"),
                           { stationId: stationId })

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

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: page._update()
            }
        }

        VerticalScrollDecorator { flickable: listView }
    }

    function _update() {
        listModel.search(coordinate, radius, fuelId)
    }

    Component.onCompleted: _update()
}
