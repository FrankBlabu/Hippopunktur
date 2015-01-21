/*
 * View.qml - GL rendering view
 *
 * Frank Blankenburg, Jan. 2015
*/

import QtQuick 1.0
import Qt3D 1.0

Viewport {
    width: 640; height: 480

    Item3D {
        id: teapot
        mesh: Mesh { source: "horse.bez" }
        effect: Effect {}
    }

}
