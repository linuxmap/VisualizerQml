import QtQuick 2.7
import QtGraphicalEffects 1.0

Item{
    id:root
    implicitWidth: 600
    implicitHeight: 480
    property real radius: 10
    Rectangle{
        id:rect
        anchors.fill:parent
        radius:root.radius
        visible: false
    }
    DropShadow{
        anchors.fill: parent
        z:-1
        horizontalOffset: 0
        verticalOffset: 0
        radius: 15
        samples: radius
        color: "#80000000"
        source: rect
        opacity:0.5
    }

}
