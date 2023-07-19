import QtQuick 2.0
import Sailfish.Silica 1.0

import de.richardliebscher.tanken 0.1


Page {
    id: page

    allowedOrientations: Orientation.All

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
                text: qsTr("Favioutes")
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
                itemHeight: Theme.itemSizeMedium
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

        VerticalScrollDecorator { flickable: content }
    }
}
