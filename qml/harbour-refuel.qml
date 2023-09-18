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

    function formatPrice(price) {
        if (isNaN(price)) {
            return ["-.--", "-"]
        }

        var str = price.toFixed(3)
        return [str.slice(0, -1), str.slice(-1)]
    }

    function formatFuel(fuel) {
        switch (fuel) {
        case FuelPriceProvider.SuperE5:
            return qsTr("Super E5")
        case FuelPriceProvider.SuperE10:
            return qsTr("Super E10")
        case FuelPriceProvider.Diesel:
            return qsTr("Diesel")
        default:
            return "<Unknown Fuel>"
        }
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
