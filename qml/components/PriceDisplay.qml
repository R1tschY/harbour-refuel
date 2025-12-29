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

Label {
    id: root

    property real value

    property color placeholderColor: Theme.secondaryColor

    height: pricePlaceholder.height
    width: pricePlaceholder.width + decimalPricePlaceholder.width

    font.pixelSize: Theme.fontSizeLarge
    font.family: dseg7.name
    color: Theme.highlightColor
    text: "-.--"

    Label {
        id: pricePlaceholder

        text: "8.88"

        font.pixelSize: root.font.pixelSize
        font.family: dseg7.name
        color: root.placeholderColor
        opacity: 0.30
        z: -1
    }

    Label {
        id: decimalPricePlaceholder

        text: "8"

        anchors.left: pricePlaceholder.right

        font.pixelSize: root.font.pixelSize * 0.7
        font.family: dseg7.name
        color: root.placeholderColor
        opacity: 0.30
    }

    Label {
        id: decimalPriceLabel

        text: "-"

        anchors.left: pricePlaceholder.right

        font.pixelSize: root.font.pixelSize * 0.7
        font.family: dseg7.name
        color: root.color
    }

    onValueChanged: {
        var v = value
        if (!v || isNaN(v)) {
            root.text = "-.--"
            decimalPriceLabel.text = "-"
        }

        var str = v.toFixed(3)
        root.text = str.slice(0, -1)
        decimalPriceLabel.text = str.slice(-1)
    }
}
