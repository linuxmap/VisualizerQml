import QtQuick 2.7
import QtQml 2.2
import QtGraphicalEffects 1.0

Rectangle {
    id:pictureGallary
    width: 800
    height: 600
    radius: 10
    property alias pictures: view.model;
    property alias currentItem: view.currentItem
    property bool popup: true
    property bool shadow: false
    property var  orientation:"vertical"
    signal itemClicked()
    MouseArea{
        anchors.fill:parent
        onWheel:{
            if(wheel.angleDelta.y>0)
            {
                view.contentY+=view.height/8
            }
            else if(wheel.angleDelta.y<0)
            {
                view.contentY-=view.height/8
            }
        }

    }
    ListView{
        id:view
        anchors.fill:parent
        orientation:pictureGallary.orientation=="vertical"?Qt.Vertical:Qt.Horizontal
        verticalLayoutDirection:ListView.BottomToTop
        anchors.margins : 10
        model: ["img.jpg","img2.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img1.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img2.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img3.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img4.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img5.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img6.png",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img7.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img8.png",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img9.jpg",
            "file:///C:\\Work\\tempProjects\\VisualizerQml\\images\\img10.jpg"

        ]
        delegate:picture
        highlightFollowsCurrentItem: true
    }

    Component{
        id:picture
        Image{
            id:image
            width:view.width
            //height:view.height
            mipmap : true
            cache:true
            asynchronous:true
            source:modelData
            fillMode: Image.PreserveAspectFit
            property bool popup: false
            property var popupItem:0

            Rectangle{
                id:picRect
                anchors.fill: parent
                color: "transparent"
                border.color: image.popup?"#ff8080":"#808080"
                border.width: image.popup?4:2
            }
            Rectangle{
                id:shadowRect
                anchors.fill: parent
                visible: false
            }
            DropShadow {
                visible: shadow
                anchors.fill: parent
                z:-1
                horizontalOffset: 0
                verticalOffset: 0
                radius: image.width/40
                samples: radius
                color: "#ff000000"
                source: picRect
                  }
            states: [
                State {
                        name: "Current"
                        //when: image.ListView.isCurrentItem
                        PropertyChanges { target: image; scale: 0.9}
                    },
            State {
                         name: "mouseEnter"
                         when: mouse.containsMouse
                         PropertyChanges { target: image; scale: 0.9}
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
                    image.ListView.view.currentIndex = index
                    itemClicked()
                    print("picture clicked")
                }
            }

        }
   }
    Image {
        id:arrow
        height: parent/20
        fillMode: Image.PreserveAspectFit
        source: "/images/arrow.png"
        rotation: 0
        anchors.right: parent.left
        anchors.verticalCenter:  parent.verticalCenter

        MouseArea {
            anchors.fill: parent;
            anchors.margins: -10
            onClicked: {
                pictureGallary.popup = !pictureGallary.popup
            }
        }

    }
//    Rectangle {
//        id: horizontalScrollDecorator
//        anchors.right: parent.right
//        anchors.margins: 2
//        color: "#c0c0c0"
//        width: 4
//        radius: 2
//        antialiasing: true
//        height: view.height * (view.height / view.contentHeight) - (width - anchors.margins) * 2
//        x:  view.contentY * (view.height / view.contentHeight)
//        NumberAnimation on opacity { id: hfade; to: 0; duration: 500 }
//        onXChanged: { opacity = 1.0; scrollFadeTimer.restart() }
//    }


    Timer { id: scrollFadeTimer; interval: 1000; onTriggered: { hfade.start();  } }
    states: [
        State {
                name: "show"
                when: popup
                PropertyChanges { target: gallary; x: gallary.parent.width-gallary.width}
                PropertyChanges { target: arrow; rotation: 270}
            },
    State {
                 name: "high"
                 when: popup===false
                 PropertyChanges { target: gallary; x: gallary.parent.width}
                 PropertyChanges { target: arrow; rotation: 90}
            }
    ]
    transitions: Transition {
        ParallelAnimation{
        NumberAnimation { easing.type: Easing.OutCubic; properties: "x"; duration: 200 }
        RotationAnimation { direction: RotationAnimation.Counterclockwise; duration: 300;}
        }
    }
}
