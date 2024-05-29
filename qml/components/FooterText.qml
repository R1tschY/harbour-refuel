import QtQuick 2.6
import Sailfish.Silica 1.0


Column {
    property alias text: label.text

    width: parent.width
    topPadding: Theme.paddingLarge
    bottomPadding: Theme.paddingMedium
    leftPadding: Theme.horizontalPageMargin
    rightPadding: Theme.horizontalPageMargin

    Label {
        id: label

        textFormat: Text.StyledText

        font.pixelSize: Theme.fontSizeSmall
        color: Theme.secondaryHighlightColor
        linkColor: Theme.secondaryColor
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
