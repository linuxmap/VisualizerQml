import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import ophelia.qml.controls 1.0
import Qt.labs.platform 1.0 as Lab
Popup {
    id:root
    //state: "closed"
    //modality:Qt.ApplicationModal
    property string fontFamily: "Verdana"
    property int leftSpace: width/10
    property int  titleHeight: height/8
    property bool shadow: true
    property alias currentCamera: cameraCombo.currentIndex
    property var camera: 0
    property var uvcUacInfo: 0
    property var mic: 0
    property alias saveDirectory:saveDirectory.text
    property string workingDirectory: "./"

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
            text:qsTr("Settings")
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
            property real textWidth: width*0.4
            RowLayout{
				id:camSwitch
                width: parent.width
                //spacing:width/10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    id:camSwitchText
                    text:qsTr("Camera:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.family: root.fontFamily
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                }

					ComboBox {
                        id:cameraCombo
                        Layout.columnSpan: 2
                        Layout.rowSpan: 1
                        Layout.fillHeight: false
                        Layout.fillWidth: true
                        model: uvcUacInfo.uvcFriendlyNames
                        currentIndex:camera.source
                        onActivated:{
                            camera.source=index
                        }
					}
			}
            RowLayout{
                id:colorSpaceList
                width: parent.width
                //spacing:width/10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("ColorSpaces:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.family: root.fontFamily
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    }

                    ComboBox {
                        id:colorSpaceCombo
                        Layout.columnSpan: 2
                        Layout.fillHeight: false
                        Layout.fillWidth: true
                        model: camera.colorSpaces
                        currentIndex: camera.currentColorSpace
                        onActivated:{
                            camera.currentColorSpace=index
                            print("colorSpaces:",camera.colorSpaces)
                            print("source:",camera.source)
                        }
                        Component.onCompleted:{
                            print("Camera:",camera)
                        }
                    }

}

            RowLayout{
                id:resolSwitch
                width: parent.width
                //spacing:width/10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Resolution:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.family: root.fontFamily
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    }

                    ComboBox {
                        id:resolCombo
                        Layout.columnSpan: 2
                        Layout.fillHeight: false
                        Layout.fillWidth: true
                        model: camera.resolutions
                        currentIndex: camera.currentResolution
                        onActivated:{
                            camera.currentResolution=index;
                        }
                    }

}
            RowLayout{
                id:micSwitch
                width: parent.width
                //spacing:width/10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Mic:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.family: root.fontFamily
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    }

                    ComboBox {
                        id:micSwitchCombo
                        Layout.columnSpan: 2
                        Layout.fillHeight: false
                        Layout.fillWidth: true
                        //model:uvcUacInfo.uacFriendlyNames
                        currentIndex: mic.currentUacIndex+1
                        property var none: [qsTr("None")]
                        onActivated:{
                            mic.setMic(index-1)
                        }
                        Component.onCompleted: {
                            model = Qt.binding(function() {
                                return none.concat(uvcUacInfo.uacFriendlyNames)
                            })
                        }
                    }
}
            RowLayout{
                id:saveDirectoryLayout
                width: parent.width
                //spacing:width/10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text{
                    text:qsTr("Save Directory:")
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.family: root.fontFamily
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredWidth: mainLayout.textWidth
                    }

                TextField {
                    id:saveDirectory
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    text: folderDialog.currentFolder//folderDialog.folder
                    placeholderText: qsTr("Browse a folder please !")
                }
                Button{
                    text: qsTr("Browse")
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    onClicked: {
                    folderDialog.visible?folderDialog.close():folderDialog.open()
                    }
                }
            }

        }

    }
        Lab.FolderDialog {
              id: folderDialog
              currentFolder: "file:///"+workingDirectory+"/photos"//saveDirectory.text
              //folder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
          }

}
