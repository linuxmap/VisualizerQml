import QtQuick 2.7
import ophelia.qml.controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
Image {
    id:image
    opacity:0
    mipmap : true
    cache:true
    asynchronous:true
    fillMode: Image.PreserveAspectFit
    property real dstX: x
    property real dstY: y
    property real dstWidth: width
    property real dstHeight: height
    property bool   shadow: true
    property alias bellRun :bell.running
    property alias popdown :popdown
    property alias popup :popup
    property var thumbnail:0


    signal popuped(var object)
    signal popdowned(var object)

    Rectangle{
        id:imageShadowRect
        anchors.fill: parent
        visible:false
    }
    RectangularGlow {
        id: effect
        z:-1
        anchors.fill: imageShadowRect
        glowRadius: 10
        spread: 0
        color: "black"
        cornerRadius: imageShadowRect.radius + glowRadius
        //opacity: 0
        //visible: false
    }
    MouseArea {
             anchors.fill: parent
             drag.target: parent
             drag.axis: Drag.XAndYAxis
             onClicked: {
                 print("clicked",mouse.button)
                 if(mouse.button===Qt.RightButton)
                 {
                     print("right clicked")
                     optionsMenu.open()
                 }
             }
    }
    Menu {
        id: optionsMenu
        transformOrigin: Menu.TopRight
        visible: false
        MenuItem {
            text: qsTr("Close")
            onTriggered: {
                if(popdown.running)popdown.complete()
                popdown.running=true
            }
        }

    }
    SequentialAnimation {
        id:popup
        running: true
        PropertyAction {target: image; property: "visible"; value:true }
        ParallelAnimation {
        NumberAnimation { target: image;property: "x"; to: dstX; duration: 200; easing.type: Easing.OutQuad }
        NumberAnimation { target: image;property: "y"; to: dstY; duration: 200; easing.type: Easing.OutQuad }
        NumberAnimation { target: image;property: "width"; to: dstWidth; duration: 200; easing.type: Easing.OutQuad }
        NumberAnimation { target: image;property: "height"; to: dstHeight; duration: 200; easing.type: Easing.OutQuad }
        NumberAnimation { target: image;property: "opacity"; to: 1; duration: 200; easing.type: Easing.OutQuad }
        }
        PropertyAction {target: image.thumbnail; property: "popup"; value:true }
//        ScriptAction { script: {popuped(image)}}
    }

    SequentialAnimation {
        id:popdown
        running: false
        ParallelAnimation {
        NumberAnimation { target: image;property: "x"; to: dstX; duration: 200; easing.type: Easing.InQuad }
        NumberAnimation { target: image;property: "y"; to: dstY; duration: 200; easing.type: Easing.InQuad }
        NumberAnimation { target: image;property: "width"; to: dstWidth; duration: 200; easing.type: Easing.InQuad }
        NumberAnimation { target: image;property: "height"; to: dstHeight; duration: 200; easing.type: Easing.InQuad }
        NumberAnimation { target: image;property: "opacity"; to: 0; duration: 200; easing.type: Easing.InQuad }
    }
        PropertyAction {target: image; property: "visible"; value:false }
        PropertyAction {target: image.thumbnail; property: "popup"; value:false }

//        ScriptAction {
//            script: {
//                image.visible=false
//                //popdowned(image)
//                //image.destroy()
//            }
//        }
    }

    SequentialAnimation {
        id:bell
        running: false
        loops:2
        NumberAnimation { target: image;property: "scale"; to: image.scale-0.1; duration: 100; easing.type: Easing.InQuad }
        NumberAnimation { target: image;property: "scale"; to: image.scale; duration: 100; easing.type: Easing.OutQuad }

    }
}


