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


BasePage {
    id: page

    coverView: Qt.resolvedUrl("../cover/FavsCover.qml")

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: page.width

            PageHeader {
                title: qsTr("Refuel")
            }

            SectionHeader {
                text: qsTr("Favorites")
                visible: favs.count > 0
            }

            ColumnView {
                id: favs
                itemHeight: Theme.itemSizeMedium
            }

            SectionHeader {
                text: qsTr("Last Searches")
                visible: lastSearches.count > 0
            }

            ColumnView {
                id: lastSearches
                itemHeight: Theme.itemSizeSmall

                model: SqlQueryModel {
                    id: lastSearchesQueryModel

                    localStorageName: database.dataBaseId

                    Component.onCompleted: {
                        exec("SELECT rowid, name, fuel_id as fuelId, latitude, longitude, distance
                              FROM last_searches
                              ORDER BY timestamp DESC")
                    }
                }

                delegate: ListItem {
                    id: listItem

                    width: parent.width
                    contentHeight: Theme.itemSizeSmall

                    RemorseItem { id: removeRemorse }

                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Forget")
                            onClicked: {
                                var idx = rowid
                                removeRemorse.execute(
                                            listItem, qsTr("Forgotten"), function() {
                                                   lastSearchesModel.removeByRowId(idx)
                                            })
                            }
                        }
                    }

                    Label {
                        id: nameLabel
                        text: name

                        anchors {
                            left: parent.left
                            leftMargin: Theme.horizontalPageMargin
                            right: detailsLabel.left
                            rightMargin: Theme.paddingSmall
                            verticalCenter: parent.verticalCenter
                        }

                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        id: detailsLabel
                        text: provider.fuelName(fuelId) + " | " + distance + " km"

                        anchors {
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }

                        color: listItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    onClicked: {
                        var coord = QtPositioning.coordinate(latitude, longitude)

                        pageStack.push(
                                    Qt.resolvedUrl("StationListPage.qml"),
                                    { fuelId: fuelId, radius: distance, coordinate: coord, name: name })

                        lastSearchesModel.add(
                                    Date.now(),
                                    "tankerkoenig",
                                    name,
                                    coord,
                                    fuelId,
                                    distance)
                    }
                }

                Component.onCompleted: {
                    lastSearchesModel.itemsChanged.connect(function() {
                        lastSearchesQueryModel.reload()
                    })
                }
            }
        }

        ViewPlaceholder {
            enabled: favs.count === 0 && lastSearches.count === 0
            text: qsTr("No searches done yet")
            hintText: qsTr("Pull down to start a search")
        }

        PullDownMenu {
            flickable: content

            MenuItem {
                text: qsTr("New search")
                onClicked: pageStack.push(Qt.resolvedUrl("SearchPage.qml"));
            }

        }

        PushUpMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"));
            }
        }

        VerticalScrollDecorator { flickable: content }
    }
}
