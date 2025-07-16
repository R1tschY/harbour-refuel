import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

CoverBackground {

    Component {
        id: delegateComponent

        BackgroundItem {
            height: Theme.itemSizeExtraSmall * 0.75
            width: parent.width

            property var priceParts: formatPrice(price)

            PriceDisplay {
                id: priceLabel

                y: Theme.paddingMedium * 1.1
                x: Theme.paddingSmall

                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                placeholderColor: Theme.secondaryHighlightColor

                mainPrice: priceParts[0]
                decimalPrice: priceParts[1]
            }

            Label {
                id: distanceLabel

                text: distance.toFixed(1) + " km"

                anchors {
                    top: nameLabel.bottom
                    left: nameLabel.left
                }

                font.pixelSize: Theme.fontSizeExtraSmall * 0.7
                color: Theme.secondaryHighlightColor
            }

            Label {
                id: nameLabel

                text: brand || name

                anchors {
                    left: priceLabel.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                }

                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.highlightColor
                truncationMode: TruncationMode.Fade
            }

            Rectangle {
                color: "red"
                opacity: 0.3
                anchors.fill: parent
            }
        }
    }

    Label {
        id: nameLabel

        width: parent.width

        text: app.coverViewPage.name
        color: Theme.highlightColor
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        id: detailsLabel

        width: parent.width
        anchors {
            top: nameLabel.bottom
        }

        text: provider.fuelName(app.coverViewPage.fuelId) + " | " + app.coverViewPage.radius + " km"
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.highlightColor
        horizontalAlignment: Text.AlignHCenter
    }

    ListView {
        width: parent.width
        anchors {
            top: detailsLabel.bottom
        }

        model: app.coverViewPage.model

        delegate: delegateComponent

        Rectangle {
            color: "red"
            opacity: 0.3
            anchors.fill: parent
        }
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
        }
    }
}
