/*
 * Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtLocation 5.4
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1
import "../components"


BasePage {
    id: page

    coverView: Qt.resolvedUrl("../cover/SearchResultsCover.qml")

    property variant coordinate
    property real radius
    property string fuelId
    property string name
    property alias model: listModel
    property bool _hasAttachedPage: false

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView

        model: StationListModel {
            id: listModel

            onStatusChanged: attachMapPage()
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

            property color primaryColor: delegate.highlighted
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
            property color secondaryColor: delegate.highlighted
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
            text: qsTr("Unable to fetch results")
            hintText: listModel.errorString
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
                onClicked: page.update()
            }
        }

        VerticalScrollDecorator { flickable: listView }
    }

    function update() {
        listModel.search(coordinate, radius, fuelId)
    }

    Component.onCompleted: update()

    onStatusChanged: attachMapPage()

    function attachMapPage() {
        if (status === PageStatus.Active) {
            if (listModel.status === StationListModel.Ready) {
                pageStack.pushAttached(
                            Qt.resolvedUrl("StationsMapPage.qml"),
                            {position: coordinate, model: listModel, radius: radius})
                page._hasAttachedPage = true;
            } else if (page._hasAttachedPage) {
                pageStack.popAttached(page)
                page._hasAttachedPage = false;
            }
        }
    }
}
