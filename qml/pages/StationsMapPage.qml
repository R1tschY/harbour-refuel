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
import QtPositioning 5.2
import QtLocation 5.4
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1
import "../components"


Page {
    id: page

    property var position
    property var model
    property var radius

    allowedOrientations: Orientation.All

    StationMap {
        id: map

        width: parent.width
        height: parent.height

        MapItemView {
            model: page.model
            delegate: mapDelegate
        }

        Component {
            id: mapDelegate

            MapQuickItem {
                property bool highlighted: false

                property color primaryColor: mapDelegate.highlighted
                                           ? Theme.highlightColor
                                           : Theme.primaryColor
                property color secondaryColor: mapDelegate.highlighted
                                             ? Theme.secondaryHighlightColor
                                             : Theme.secondaryColor
                property color backgroundColor: Theme.overlayBackgroundColor

                coordinate: coord
                zoomLevel: 0
                anchorPoint: Qt.point(5, 5)
                sourceItem: Item {
                    Rectangle {
                        color: colorStation(price)
                        width: 10
                        height: 10
                    }

                    Rectangle {
                        color: colorStation(price)
                        y: 5
                        width: priceLabel.width + Theme.paddingSmall
                        height: Theme.paddingMedium
                    }

                    Rectangle {
                        color: backgroundColor
                        y: 12
                        width: priceLabel.width + Theme.paddingSmall
                        height: priceLabel.height + Theme.paddingSmall
                        radius: Theme.paddingSmall / 2

                        PriceDisplay {
                            id: priceLabel

                            x: Theme.paddingSmall / 2
                            y: Theme.paddingSmall / 2

                            property var priceParts: formatPrice(price)

                            color: primaryColor
                            placeholderColor: secondaryColor

                            font.pixelSize: Theme.fontSizeExtraSmall

                            mainPrice: priceParts[0]
                            decimalPrice: priceParts[1]
                        }
                    }
                }
            }
        }

        MapCircle {
            center: position
            color: "#FF0000"
            radius: page.radius * 1000
            smooth: true
            antialiasing: true
            opacity: 0.1
            border.width: 0
        }

        center: position
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            map.plugin = locationPlugin
            map.zoomLevel = 13

            console.debug(
                        "supportedMapTypes len=" + map.supportedMapTypes.length)
            if (Qt.locale().name.indexOf("de_") === 0
                    && map.supportedMapTypes.length === 2) {
                console.debug("Set German map type")
                map.activeMapType = map.supportedMapTypes[0]
            }
        }
    }

    function colorStation(price) {
        var diff = price - model.lowestPrice;
        if (diff < 0.005) {
            return "#3fcc00";
        } else if (diff < 0.015) {
            return "#96d800";
        } else if (diff < 0.025) {
            return "#dee100";
        } else if (diff < 0.055) {
            return "#e5ca00";
        } else if (diff < 0.105) {
            return "#ee8800";
        } else {
            return "#a70000";
        }
    }
}
