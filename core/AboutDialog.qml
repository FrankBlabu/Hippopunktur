/****************************************************************************
** AboutDialog.qml
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

import QtQuick 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.2

MessageDialog {
    icon: StandardIcon.Information
    title: "About"
    text: qsTr ("<b>Hippopunktur</b>")
    informativeText: qsTr ("Akkupunkturpunkte beim Pferd")
}
