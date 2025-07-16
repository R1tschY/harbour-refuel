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
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1


Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: page.width

            PageHeader {
                title: qsTr("About Refuel")
            }

            Image {
                id: icon

                width: Theme.itemSizeExtraLarge
                height: Theme.itemSizeExtraLarge
                anchors.horizontalCenter: parent.horizontalCenter

                source: "image://theme/harbour-refuel"
            }

            Item {
                width: parent.width
                height: Theme.paddingMedium
            }

            Label {
                text: qsTr("Search for fuel station prices")

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeMedium
            }

            Item {
                width: parent.width
                height: Theme.paddingMedium
            }

            Label {
                text: qsTr("Version %1").arg(Qt.application.version)

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                text: qsTr("Copyright 2023 Richard Liebscher")

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeSmall
            }

            Item {
                width: parent.width
                height: Theme.paddingLarge
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTr("Visit on GitHub")
                onClicked: Qt.openUrlExternally("https://github.com/R1tschY/harbour-sailfishconnect")
            }

            SectionHeader {
                text: qsTr("Licence")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: "This program is free software: you can redistribute it and/or modify" +
                    "it under the terms of the GNU General Public License as published by " +
                    "the Free Software Foundation, either version 3 of the License, or " +
                    "(at your option) any later version. <br/><br/>" +

                    "This program is distributed in the hope that it will be useful, " +
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of " +
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " +
                    "GNU General Public License for more details.<br/><br/>" +

                    "You should have received a copy of the GNU General Public License " +
                    "along with this program. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
                textFormat: Text.StyledText
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                linkColor: Theme.primaryColor
                onLinkActivated: Qt.openUrlExternally(link)
            }

            SectionHeader {
                text: qsTr("Credits")
            }

            Column {
                spacing: Theme.paddingMedium
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin

                Label {
                    text: qsTr("
<h4>Map data</h4>
is licensed under the <a href='https://opendatacommons.org/licenses/odbl/'>Open Data Commons Open Database License (ODbL)</a><br/><br/>
Copyright <a href='https://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors<br/>

<h4>Geocoding</h4>
using <a href='https://photon.komoot.io/'>Photon</a><br/>

<h4>DSEG</h4>
is licensed under the <a href='http://scripts.sil.org/OFL'>SIL Open Font License 1.1</a><br/><br/>
Copyright 2020 <a href='https://www.keshikan.net'>keshikan</a><br/>
")

                    width: parent.width

                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeSmall
                    linkColor: Theme.primaryColor
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }

            Item {
                width: parent.width
                height: Theme.paddingLarge
            }
        }

        VerticalScrollDecorator { flickable: content }
    }
}
