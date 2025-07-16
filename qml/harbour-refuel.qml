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
import QtLocation 5.3
import Sailfish.Silica 1.0
import Nemo.Notifications 1.0
import "pages"
import "components"

import de.richardliebscher.refuel 0.1

ApplicationWindow {
    id: app

    initialPage: Component { HomePage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    property Page coverViewPage

    Notification {
        id: feedback

        function info(text) {
            feedback.previewSummary = text
            feedback.show()
        }
    }

    FontLoader {
        id: dseg7
        source: "/usr/share/harbour-refuel/fonts/DSEG7Modern-Regular.ttf"
    }

    Plugin {
        id: locationPlugin
        name: "osmimproved"

        PluginParameter {
            name: "osmimproved.useragent"
            value: "Refuel Sailfish OS/0.1 Qt/5.6.3" // TODO: replace versions
        }
        PluginParameter {
            name: "osmimproved.geocoder"
            value: "photon"
        }
    }

    TankerKoenigProvider {
        id: provider

        userAgent: "Refuel Sailfish OS/0.1 Qt/5.6.3" // TODO: replace versions
    }

    DbConnection {
        id: database

        dataBaseId: "config"
    }

    LastSearchesModel {
        id: lastSearchesModel
        db: database
    }

    Connections {
        target: pageStack

        onCurrentPageChanged: {
            var page = pageStack.find(function(page) { return !!page.coverView })
            if (page) {
                app.cover = page.coverView
                app.coverViewPage = page
            } else {
                app.cover = Qt.resolvedUrl("cover/FavsCover.qml")
                app.coverViewPage = null
            }
        }
    }

    function formatPrice(price) {
        if (isNaN(price)) {
            return ["-.--", "-"]
        }

        var str = price.toFixed(3)
        return [str.slice(0, -1), str.slice(-1)]
    }

    function formatAddress(address) {
        var res = ""
        // TODO: check country code
        if (address.postcode) {
            res += address.postcode
            res += " "
        }
        if (address.city) {
            res += address.city
        }
        if (address.state) {
            if (res.length !== 0) {
                res += ", "
            }
            res += address.state
        }
        if (address.country) {
            if (res.length !== 0) {
                res += ", "
            }
            res += address.country
        }

        return res
    }

    function geoUri(coordinate) {
        return "geo:" + coordinate.latitude + "," + coordinate.longitude
    }
}
