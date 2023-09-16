import QtQuick 2.0
import Sailfish.Silica 1.0

Label {
    id: root

    property color color: Theme.highlightColor

    property alias mainPrice: root.text
    property alias decimalPrice: decimalPriceLabel.text

    property var primaryColor: highlighted
                               ? Theme.highlightColor
                               : Theme.primaryColor
    property var secondaryColor: highlighted
                                 ? Theme.secondaryHighlightColor
                                 : Theme.secondaryColor

    height: pricePlaceholder.height
    width: pricePlaceholder.width + decimalPricePlaceholder.width

    font.pixelSize: Theme.fontSizeLarge
    font.family: dseg7.name

    Label {
        id: pricePlaceholder

        text: "8.88"

        font.pixelSize: Theme.fontSizeLarge
        font.family: dseg7.name
        color: root.color
        opacity: 0.30
        z: -1
    }

    Label {
        id: decimalPricePlaceholder

        text: "8"

        anchors.left: pricePlaceholder.right

        font.pixelSize: Theme.fontSizeSmall
        font.family: dseg7.name
        color: root.color
        opacity: 0.30
    }

    Label {
        id: decimalPriceLabel

        anchors.left: pricePlaceholder.right

        font.pixelSize: Theme.fontSizeSmall
        font.family: dseg7.name
        color: root.color
    }
}
