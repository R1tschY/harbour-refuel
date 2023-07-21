import QtQuick 2.0
import QtLocation 5.3
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1

Page {
    allowedOrientations: Orientation.All

    Plugin {
        id: geocodingPlugin
        name: "osmimproved"

        PluginParameter {
            name: "useragent";
            value: "Refuel Sailfish OS/0.1 QtLocation/" + qVersion
        }
    }

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
                placeholderText: qsTr("Address")

                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-search"
                EnterKey.onClicked: {
                    geocodeModel.query = searchField.text
                    geocodeModel.update()
                }
            }
        }

        model: GeocodeModel {
            id: geocodeModel
            plugin: geocodingPlugin
            autoUpdate: false
            limit: 15
        }

        delegate: BackgroundItem {
            id: delegateItem

            onClicked: {
                pageStack.pop()
            }

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

        ViewPlaceholder {
            enabled: geocodeModel.status === GeocodeModel.Error
            text: geocodeModel.errorString
        }

        ViewPlaceholder {
            enabled: listView.count === 0
                     && geocodeModel.status === GeocodeModel.Ready
            text: qsTr("Nothing found") + geocodeModel.error
        }

        ViewPlaceholder {
            enabled: geocodeModel.status === GeocodeModel.Null
            text: qsTr("Start search")
        }

        BusyLabel {
            running: geocodeModel.status === GeocodeModel.Loading
        }

        VerticalScrollDecorator { flickable: listView }
    }
}
