import QtQuick 2.0
import QtQuick.Controls 2.2
import ophelia.qml.controls 1.0
import QtQuick.Window 2.2
//import QtGraphicalEffects 1.0

Rectangle {
    id:root
    width: Screen.desktopAvailableWidth/1.5
    height: Screen.desktopAvailableHeight/1.2
    //property string name: value
    color: theme?"white":"#ff101010"
    property int theme: 1
    property bool shadow: true
    //property var anotationDialog: toolBar._anotaionDialog
    property bool anotationEnable:toolBar.anotationEnable
    //color:"#ff505050"
    // The checkers background
    property bool pipMode: false
FocusScope
{
    anchors.fill:parent
    InputItem{
        id:inputItem
        anchors.fill:parent
        target:cameraRect
        focus: true
        Keys.onPressed: {
            switch(event.key)
            {
            case Qt.Key_Left:
                break;
            case Qt.Key_Space:
                camera.togglePause();
                console.log("Pause");
                break;
            case Qt.Key_R:
                animations.running=!animations.running;
                break;
            default:
                break;
            }
        }

        onRightClicked:{
            optionsMenu.x=x;
            optionsMenu.y=y;
            optionsMenu.visible=true
        }
    }

}

Rectangle{
    id:cameraRect
    x:0
    y:0
    width:parent.width
    height: parent.height
    //anchors.verticalCenter: parent.verticalCenter
    visible: true
   // color: "black"
//    flags:Qt.Widget|Qt.FramelessWindowHint
   // scale:0.8
    transform: [
        Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: cameraRect.width / 2; origin.y: cameraRect.height / 2; },
        Translate { id: txOut;
            x: -cameraRect.width / 2;
            y: -cameraRect.height / 2
        },
        Scale { id: camScale; },
        Translate { id: txIn; x: cameraRect.width / 2; y: cameraRect.height / 2 }
    ]

    UvcUacInfo{
        id:uvcUacInfo
    }
    UvcCamera{
        id:camera
        displayItem:videoOutput.renderer
        uvcControler{
           id:cameraControler
        }
        photoDirectory:settingsDialog.saveDirectory
    }
    FFMedia{
        id:mic
        property int currentUacIndex: -1
        function setMic(index)
        {
            if(index===currentUacIndex)return
            if(index===-1){
                mic.stop();
                currentUacIndex=index
                return
            }
            source="audio="+uvcUacInfo.uacFriendlyNames[index]
            if(!mic.play())
            {
                currentUacIndex=index
            }
        }
    }
    VideoDisplay {
        id: videoOutput
        anchors.fill: parent
        //opacity: 1
//        Rectangle{
//            id:camShadowRect
//            anchors.fill: parent
//            visible:false
//        }
//        RectangularGlow {
//            id: effect
//            z:-1
//            anchors.fill: camShadowRect
//            glowRadius: 10
//            spread: 0
//            color: "black"
//            cornerRadius: camShadowRect.radius + glowRadius
//           //opacity: 1
//            visible: false
//        }
    }

    AnotationLayer{
        id:anotationLayer
        anchors.fill: videoOutput
        enabled:root.anotationEnable
        penWidth: anotaionDialog.penWidth
        //visible: true
    }
}
ImageViwer{
    id:popImage
    backgroundItem: cameraRect
}

Rectangle{
    id:toolsBackground
    anchors.bottom: parent.bottom
    anchors.bottomMargin: radius
    height: parent.height/15
    width: parent.width/20*toolBar.buttonCount
    anchors.horizontalCenter: parent.horizontalCenter
    visible: false
}

Tools{
    id:toolBar
    anchors.fill:toolsBackground
    settingsDialog:settingsDialog
    picturesDialog: picturesDialog
    anotaionDialog: anotaionDialog
    anotationlayer: anotationLayer
    adjustDialog:adjustDialog
    visible: !pipMode
}
PictureFlow{
    id:picturesDialog
    x:(parent.width-width)/2
    y:toolBar.y-height*1.05
    width: parent.width*0.95
    height: parent.height/6
    padding:0
    pictureDirectory: settingsDialog.saveDirectory
    onImageClicked: {
        popImage.setSourceImage(image)
    }
}
AdjustDialog{
    id:adjustDialog
    x:toolBar.x+toolBar.width-width
    y:toolBar.y-height*1.05
    width:parent.width/4
    height: parent.height/3
    camera:cameraControler
}
SettingsDialog{
    id:settingsDialog
    x:toolBar.x+toolBar.width-width
    y:toolBar.y-height*1.05
    width:parent.width/3<450?450:parent.width/3
    height: parent.height/3<500?500:parent.height/3
    shadow:root.shadow
    currentCamera:camera.source
    camera:camera
    mic:mic
    uvcUacInfo:uvcUacInfo
    workingDirectory:workingdirectory
}
Anotation{
    id:anotaionDialog
    x:toolBar.x
    y:toolBar.y-height
    width:300//parent.width/4
    height: 400//parent.height/4
    anotationlayer:anotationLayer
}

    Menu {
        id: optionsMenu
        transformOrigin: Menu.TopRight
        visible: false
        MenuItem {
            text: "Settings"
            onTriggered: {
            }
        }
        MenuItem {
            text: "About"
            onTriggered: aboutDialog.open()
        }
        MenuItem {
            text: "Settings"
            onTriggered: settingsDialog.open()
        }
        MenuItem {
            text: "About"
        }
        MenuItem {
            id:pipAction
            text: qsTr("Enter PiP mode")
            property real savedX: 0
            property real savedY: 0
            property real savedWidth: 0
            property real savedHeight: 0
            onTriggered:{
                if(!pipMode)
                {
                    savedX = window.x
                    savedY = window.y
                    savedWidth = window.width
                    savedHeight = window.height
                    window.flags|=Qt.WindowStaysOnTopHint|Qt.FramelessWindowHint
                    window.width=600
                    window.height=480
                    window.x=Screen.desktopAvailableWidth*0.98-window.width
                    window.y=Screen.desktopAvailableHeight*0.02
                    pipMode=true
                    pipAction.text=qsTr("Exit Pip mode")
                }
                else{
                    window.flags&=~(Qt.WindowStaysOnTopHint|Qt.FramelessWindowHint)
                    window.x=savedX
                    window.y=savedY
                    window.width=savedWidth
                    window.height=savedHeight
                    pipMode=false
                    pipAction.text=qsTr("Enter Pip mode")
                }
            }
        }
        MenuItem {
            text: "Exit"
            onTriggered: Qt.quit()
        }
    }
    Component.onCompleted: {
        camera.play()
        if(uvcUacInfo.uacFriendlyNames.length>0
                &&mic.currentUacIndex>=0
                &&mic.currentUacIndex<uvcUacInfo.uacFriendlyNames.length){
            mic.source="audio="+uvcUacInfo.uacFriendlyNames[mic.currentUacIndex]
            mic.play()
        }
        console.debug("workingDirectory:","")
    }
    // Just to show something interesting
    SequentialAnimation {
        id:animations
        PauseAnimation { duration: 3000 }
        NumberAnimation{ target:txOut; property: "x"; to:-cameraRect.width*camScale.xScale*1.5; duration:1000; easing.type: Easing.InOutBack }
        PauseAnimation { duration: 1000 }
        NumberAnimation{ target:txOut;property: "x"; to:-cameraRect.width*camScale.xScale/2; duration:1000; easing.type: Easing.InOutBack }
        PauseAnimation { duration: 1000 }

        ParallelAnimation {
            NumberAnimation { target: camScale; property: "xScale"; to: 0.5; duration: 1000; easing.type: Easing.InOutBack }
            NumberAnimation { target: camScale; property: "yScale"; to: 0.5; duration: 1000; easing.type: Easing.InOutBack }
        }
        PropertyAction{target: rotation; property: "axis.x";value: 1;}
        PropertyAction{target: rotation; property: "axis.y";value: 0;}
        PropertyAction{target: rotation; property: "axis.z";value: 0;}
        PropertyAction{target: rotation; property: "angle";value: 0;}

        NumberAnimation { target: rotation; property: "angle"; to: 1080; duration: 1000; easing.type: Easing.InOutCubic }
        PauseAnimation { duration: 1000 }

        PropertyAction{target: rotation; property: "axis.x";value: 0;}
        PropertyAction{target: rotation; property: "axis.y";value: 1;}
        PropertyAction{target: rotation; property: "axis.z";value: 0;}
        PropertyAction{target: rotation; property: "angle";value: 0;}

        NumberAnimation { target: rotation; property: "angle"; to: 80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: -80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: 0; duration: 1000; easing.type: Easing.InOutCubic }
        PauseAnimation { duration: 1000 }

        PropertyAction{target: rotation; property: "axis.x";value: 0;}
        PropertyAction{target: rotation; property: "axis.y";value: 0;}
        PropertyAction{target: rotation; property: "axis.z";value: 1;}
        PropertyAction{target: rotation; property: "angle";value: 0;}
        NumberAnimation { target: rotation; property: "angle"; to: 1080; duration: 1000; easing.type: Easing.InOutCubic }

        NumberAnimation { target: cameraRect; property: "opacity"; to: 0.0; duration: 1000; easing.type: Easing.InOutCubic }
        PauseAnimation { duration: 1000 }
        NumberAnimation { target: cameraRect; property: "opacity"; to: 1.0; duration: 1000; easing.type: Easing.InOutCubic }
        ParallelAnimation {
            NumberAnimation { target: camScale; property: "xScale"; to: 1.0; duration: 1000; easing.type: Easing.InOutBack }
            NumberAnimation { target: camScale; property: "yScale"; to: 1.0; duration: 1000; easing.type: Easing.InOutBack }

        }
        running: false
        loops: Animation.Infinite
    }

    Rectangle{
        id:pipBorder
        anchors.fill: parent
        border.width: 5
        border.color: "#80000000"
        color: "transparent"
        visible: pipMode
    }


}
