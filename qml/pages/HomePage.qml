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
import "../components"


BasePage {
    id: page

    coverView: Qt.resolvedUrl("../cover/FavsCover.qml")

    property bool dirtyLastSearches: false
    property bool dirtyFavs: false

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

                model: SqlQueryModel {
                    id: favsQueryModel

                    localStorageName: database.dataBaseId

                    Component.onCompleted: {
                        exec("SELECT rowid, id as stationId, brand, name, address, fuel_id as fuelId
                              FROM favourites
                              ORDER BY `order` ASC")
                    }
                }

                delegate: ListItem {
                    id: fav

                    contentHeight: Theme.itemSizeMedium

                    property var priceParts: formatPrice(0)

                    property color primaryColor: fav.highlighted
                                               ? Theme.highlightColor
                                               : Theme.primaryColor
                    property color secondaryColor: fav.highlighted
                                                 ? Theme.secondaryHighlightColor
                                                 : Theme.secondaryColor

                    PriceDisplay {
                        id: favPriceLabel

                        y: Theme.paddingMedium * 1.1
                        x: Theme.horizontalPageMargin

                        color: fav.primaryColor
                        placeholderColor: fav.secondaryColor

                        mainPrice: priceParts[0]
                        decimalPrice: priceParts[1]
                    }

                    Label {
                        id: favDetailsLabel

                        text: provider.fuelName(fuelId) + "  •  " + address

                        anchors {
                            bottom: parent.bottom
                            bottomMargin: Theme.paddingSmall
                            left: favPriceLabel.left
                        }

                        font.pixelSize: Theme.fontSizeSmall
                        color: secondaryColor
                    }

                    Label {
                        id: favNameLabel

                        text: brand || name

                        anchors {
                            baseline: favPriceLabel.baseline
                            baselineOffset: -Theme.paddingSmall
                            left: favPriceLabel.right
                            leftMargin: Theme.paddingMedium
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                        }

                        font.pixelSize: Theme.fontSizeLarge
                        color: primaryColor
                        truncationMode: TruncationMode.Fade
                    }

                    onClicked: pageStack.push(
                                   Qt.resolvedUrl("StationDetailsPage.qml"),
                                   { stationId: stationId })

                    menu: ContextMenu {
                         MenuItem {
                             text: qsTr("Remove")
                             onClicked: {
                                 var stationId_ = stationId
                                 var fuelId_ = stationId
                                 fav.remorseDelete(function() {
                                     favsModel.remove(
                                                "tankerkoenig",
                                                stationId_,
                                                fuelId_)
                                 })
                             }
                         }
                     }
                }
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
                        text: provider.fuelName(fuelId) + "  •  " + distance + " km"

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

    Connections {
        target: favsModel

        onItemsChanged: {
            if (status === PageStatus.Active) {
                favsQueryModel.reload()
            } else {
                dirtyFavs = true
            }
        }
    }

    Connections {
        target: lastSearchesModel

        onItemsChanged: {
            if (status === PageStatus.Active) {
                lastSearchesQueryModel.reload()
            } else {
                dirtyLastSearches = true
            }
        }
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (dirtyFavs) {
                favsQueryModel.reload()
                dirtyFavs = false;
            }
            if (dirtyLastSearches) {
                lastSearchesQueryModel.reload()
                dirtyLastSearches = false;
            }
        }
    }
}
