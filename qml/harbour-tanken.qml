import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    initialPage: Component { HomePage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    function formatPrice(price) {
        var str = price.toFixed(3)
        return [str.slice(0, -1), str.slice(-1)]
    }
}
