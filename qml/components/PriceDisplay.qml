import QtQuick 2.0
import Sailfish.Silica 1.0

Label {
    id: root

    property alias mainPrice: root.text
    property alias decimalPrice: decimalPriceLabel.text

    property color placeholderColor: Theme.secondaryColor

    height: pricePlaceholder.height
    width: pricePlaceholder.width + decimalPricePlaceholder.width

    font.pixelSize: Theme.fontSizeLarge
    font.family: dseg7.name
    color: Theme.highlightColor

    Label {
        id: pricePlaceholder

        text: "8.88"

        font.pixelSize: root.font.pixelSize
        font.family: dseg7.name
        color: root.placeholderColor
        opacity: 0.30
        z: -1
    }

    Label {
        id: decimalPricePlaceholder

        text: "8"

        anchors.left: pricePlaceholder.right

        font.pixelSize: root.font.pixelSize * 0.7
        font.family: dseg7.name
        color: root.placeholderColor
        opacity: 0.30
    }

    Label {
        id: decimalPriceLabel

        anchors.left: pricePlaceholder.right

        font.pixelSize: root.font.pixelSize * 0.7
        font.family: dseg7.name
        color: root.color
    }
}
