import QtQuick 2.0

Rectangle{
    id:root
    width:60
    height: 60
    radius: width/20
    color: "blue"
    border.width: 0
    states: [
        State {
                name: "Current"
                //when: image.ListView.isCurrentItem
                PropertyChanges { target: root; scale: 1}
            },
    State {
                 name: "mouseEnter"
                 when: mouse.containsMouse
                 PropertyChanges { target: root; scale: 0.9}
            }
    ]

    transitions: Transition {
        NumberAnimation { easing.type: Easing.OutCubic; properties: "scale"; duration: 300 }
    }


    MouseArea {
        id:mouse
        anchors.fill: parent
        hoverEnabled:true
        onClicked:{
        }

}
}
