import QtQuick 2.6
import QtLocation 5.3
import Sailfish.Silica 1.0
import "../components"

import de.richardliebscher.refuel 0.1

Page {
    id: page

    allowedOrientations: Orientation.All

    property var location

    signal selected(var location)

    SilicaListView {
        id: listView

        anchors.fill: parent

        header: Column {
            width: parent.width

            PageHeader {
                title: qsTr("Choose position")
            }

            SearchField {
                id: searchField

                inputMethodHints: Qt.ImhNoPredictiveText
                placeholderText: qsTr("Search address")
                focus: true

                EnterKey.enabled: listView.count > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: listView.accept(geocodeModel.get(0))

                onTextChanged: geocodeModel.search(searchField.text)
            }

            Component.onCompleted: searchField.forceActiveFocus()
        }

        footer: FooterText {
            text: qsTr("© <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors")
            visible: listView.count > 0
                     && geocodeModel.status === GeocodeModel.Ready
        }

        model: GeocodeModel {
            id: geocodeModel
            plugin: locationPlugin
            autoUpdate: false
            bounds: provider.boundingBox
            limit: 10

            function search(query) {
                geocodeModel.query = query
                if (query) {
                    geocodeModel.update()
                } else {
                    geocodeModel.reset()
                }
            }
        }

        delegate: BackgroundItem {
            id: delegateItem

            onClicked: listView.accept(locationData)

            Label {
                id: mainLabel

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                }

                truncationMode: TruncationMode.Fade
                text: locationData.address.text
                highlighted: delegateItem.highlighted
            }

            Label {
                id: descriptionLabel

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    top: mainLabel.bottom
                }

                font.pixelSize: Theme.fontSizeExtraSmall
                truncationMode: TruncationMode.Fade
                text: formatAddress(locationData.address)
                highlighted: delegateItem.highlighted
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            }
        }

        function accept(location) {
            page.location = location
            page.selected(location)
            pageStack.pop()
        }

        ViewPlaceholder {
            enabled: geocodeModel.status === GeocodeModel.Error
            text: geocodeModel.errorString
        }

        ViewPlaceholder {
            enabled: listView.count === 0
                     && geocodeModel.status === GeocodeModel.Ready
            text: qsTr("Nothing found")
        }

        ViewPlaceholder {
            enabled: geocodeModel.status === GeocodeModel.Null
            text: qsTr("Type to start search")
        }

        BusyLabel {
            running: geocodeModel.status === GeocodeModel.Loading
        }

        VerticalScrollDecorator { flickable: listView }
    }
}
