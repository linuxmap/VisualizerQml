import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import ophelia.qml.controls 1.0

Popup {
    id:root
    property string fontFamily: "Verdana"
    property int leftSpace: width/10
    property int  titleHeight: height/8
    property bool shadow: true
    property var camera: 0
    property var uvcUacInfo: 0
    property var mic: 0

    Item{
        id:title
        anchors.top:parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: root.titleHeight
        Text{
            opacity:1
            x:root.leftSpace
            anchors.top:parent.top
            anchors.topMargin:height/2
            text:qsTr("Adjust")
            font.family: root.fontFamily
            font.bold: true
            textFormat: Text.AutoText
        }
    }
        Item{
        id:body
        anchors.top:title.bottom
        anchors.bottom:parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        ColumnLayout{
            id:mainLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: root.leftSpace
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: root.leftSpace
            property real textWidth: width*0.3
            RowLayout{
                id:zoomLayout
                width: parent.width
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Zoom:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    font.family: root.fontFamily
                    }
                Button{
                    id:zoomIn
                    text: qsTr("Tele")
                    Layout.alignment:Qt.AlignCenter
                    Layout.maximumWidth : 100
                    Layout.maximumHeight : 80
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    //autoRepeat:true
                    onPressed: {
                        console.debug("pressed")
                        camera.zoomIn()
                    }
                    onReleased: {
                        console.debug("released")
                        camera.zoomStop()
                    }
                }
                Button{
                    id:zoomOut
                    text: qsTr("Wide")
                    Layout.alignment:Qt.AlignCenter
                    Layout.maximumWidth : 100
                    Layout.maximumHeight : 80
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    //autoRepeat:true
                    onPressed: {
                        console.debug("pressed")
                        camera.zoomOut()
                    }
                    onReleased: {
                        console.debug("released")
                        camera.zoomStop()
                    }

                }
            }
            RowLayout{
                id:rotateLayout
                width: parent.width
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Rotate:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    font.family: root.fontFamily
               }
                Button{
                    id:rotateLeft
                    Layout.maximumWidth : 100
                    Layout.maximumHeight : 80
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onPressed: {
                        console.debug("pressed")
                        camera.zoomIn()
                    }
                    onReleased: {
                        console.debug("released")
                        camera.zoomStop()
                    }
                }
                Button{
                    id:rotateRight
                    Layout.maximumWidth : 100
                    Layout.maximumHeight : 80
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    //autoRepeat:true
                    onPressed: {
                        console.debug("pressed")
                        camera.zoomOut()
                    }
                    onReleased: {
                        console.debug("released")
                        camera.zoomStop()
                    }

                }

            }
            RowLayout{
                id:brightness
                width: parent.width
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Brightness:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    font.family: root.fontFamily
                    }
                    Slider {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
            }
            RowLayout{
                id:sharpness
                width: parent.width
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Sharpness:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    font.family: root.fontFamily
                }

                Slider {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }
        }

    }
}
