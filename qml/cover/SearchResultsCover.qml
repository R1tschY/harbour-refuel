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
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1
import "../components"

CoverBackground {

    Component {
        id: delegateComponent

        BackgroundItem {
            height: Theme.itemSizeExtraSmall * 0.75
            width: parent.width

            property var priceParts: formatPrice(price)

            PriceDisplay {
                id: priceLabel

                y: Theme.paddingMedium * 1.1
                x: Theme.paddingSmall

                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                placeholderColor: Theme.highlightColor

                mainPrice: priceParts[0]
                decimalPrice: priceParts[1]
            }

            Label {
                id: nameLabel

                text: distance.toFixed(1) + " km: " + (brand || name)

                anchors {
                    left: priceLabel.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                    verticalCenter: priceLabel.verticalCenter

                }

                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                truncationMode: TruncationMode.Fade
            }
        }
    }

    Label {
        id: nameLabel

        width: parent.width

        text: app.coverViewPage.name
        color: Theme.highlightColor
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        id: detailsLabel

        width: parent.width
        anchors {
            top: nameLabel.bottom
        }

        text: provider.fuelName(app.coverViewPage.fuelId) + " · " + app.coverViewPage.radius + " km"
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.highlightColor
        horizontalAlignment: Text.AlignHCenter
    }

    ListView {
        id: listView

        width: parent.width
        height: Theme.itemSizeExtraSmall * 0.75 * 10
        anchors {
            top: detailsLabel.bottom
        }

        model: app.coverViewPage.model

        delegate: delegateComponent
    }

    Label {
        anchors {
            top: detailsLabel.bottom
            topMargin: Theme.paddingLarge
            left: parent.left
            leftMargin: Theme.paddingLarge
        }

        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeLarge
        text: qsTr("Fetching...")
        visible: app.coverViewPage.model.status === StationListModel.Loading
    }

    Label {
        anchors {
            top: detailsLabel.bottom
            topMargin: Theme.paddingLarge
            left: parent.left
            leftMargin: Theme.paddingLarge
        }

        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeLarge
        text: qsTr("Unable to fetch")
        visible: app.coverViewPage.model.status === StationListModel.Error
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: app.coverViewPage.update();
        }
    }
}
