/****************************************************************************
* Explorer.qml
*
* Frank Blankenburg, Jan. 2014
****************************************************************************/

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import com.blankenburg.hippopunktur 1.0

Rectangle {
    id: root

    TableView {
        id: tableView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        frameVisible: false
        sortIndicatorVisible: true

        TableViewColumn {
            id: titleColumn
            title: "Points"
            role: "point"
            movable: false
            resizable: false
            width: tableView.viewport.width
        }

        model: sourceModel

        ListModel {
            id: sourceModel

            ListElement {
                point: "BI1"
            }
            ListElement {
                point: "BI2"
            }
            ListElement {
                point: "BI3"
            }
        }
    }
}

