import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string stationId
    property string stationName
    property string stationBrand
    property string stationAddress
    property bool stationIsOpen

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: page.width

            PageHeader {
                title: stationName
            }

            Label {
                text: stationBrand
            }

            Label {
                text: stationAddress
            }


            Label {
                text: stationIsOpen ? qsTr("Open") : qsTr("Closed")
            }


            SectionHeader {
                text: qsTr("Prices")
                visible: stationIsOpen
            }

//           ColumnView {
//               model: [
//                   { name: "Super E5", price: station. }
//               ]
//           }
        }

        PullDownMenu {
            flickable: content

            MenuItem {
                text: qsTr("Update")
                onClicked: console.log("TODO")
            }
        }

        VerticalScrollDecorator { flickable: content }
    }
}
