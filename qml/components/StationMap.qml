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

Map {
    property Item flickable
    property bool mapReady: false

    gesture {
        enabled: true
    }

    MouseArea {
        anchors.fill: parent

        onPressed: {
            if (flickable) {
                flickable.interactive = false
            }
            page.enabled = false
        }
        onReleased: {
            if (flickable) {
                flickable.interactive = true
            }
            page.enabled = true
        }
    }

    onSupportedMapTypesChanged: mapReady = true
}
