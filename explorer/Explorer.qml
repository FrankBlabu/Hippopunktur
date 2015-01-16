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

                height: text.height + 10
                width: parent.width

                Rectangle {
                    id: background
                    anchors.fill: parent
                    color: selected ? "lightgray" : "gray"
                    radius: 3
                }

                Column {
                    id: text
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5

                    Text {
                        id: upper_text
                        width: parent.width
                        text: name
                        color: selected ? "gray" : "lightgray"
                        wrapMode: Text.Wrap
                    }
                    Text {
                        id: lower_text
                        width: parent.width
                        text: description
                        color: selected ? "gray" : "lightgray"
                        wrapMode: Text.Wrap
                        visible: selected
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        selected = !selected
                        explorer.currentIndex = index
                    }
                }
            }
        }

        model: explorer_model
        delegate: point_delegate
        focus: true
    }

}

