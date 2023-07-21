import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    initialPage: Component { ChooseLocationDialog { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    function formatPrice(price) {
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
}
