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

    ListView {
        id: explorer

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Component {
            id: point_delegate

            Rectangle {
                id: wrapper
                height: upper_text.height + lower_text.height + separator.height
                width: parent.width

                Rectangle {
                    id: separator
                    width: parent.width
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    color: "lightgray"
                }

                Column {
                    Text {
                        id: upper_text
                        text: name
                        color: selected ? "red" : "black"
                    }
                    Text {
                        id: lower_text
                        text: description
                        color: selected ? "red" : "black"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: { selected = !selected; }
                }
            }
        }

        model: explorer_model
        delegate: point_delegate
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true
    }

}

