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

import QtQuick 2.6
import Sailfish.Silica 1.0


Column {
    property alias text: label.text

    width: parent.width
    topPadding: Theme.paddingLarge
    bottomPadding: Theme.paddingMedium
    leftPadding: Theme.horizontalPageMargin
    rightPadding: Theme.horizontalPageMargin

    Label {
        id: label

        textFormat: Text.StyledText

        font.pixelSize: Theme.fontSizeSmall
        color: Theme.secondaryHighlightColor
        linkColor: Theme.secondaryColor
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
