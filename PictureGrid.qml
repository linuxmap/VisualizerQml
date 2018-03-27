import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import Qt.labs.folderlistmodel 2.1
Popup{
    id:root
    property string pictureDirectory: undefined
    GridView{
        id:gridView
        anchors.fill:parent
        anchors.margins : 20
        cellWidth:width/4
        cellHeight: cellWidth
        verticalLayoutDirection:GridView.BottomToTop
        model:FolderListModel{
            folder:root.pictureDirectory// "file:/C:\Work\tempProjects\VisualizerQml\images"
            nameFilters: [ "*.png", "*.jpg" ]
            showDirs : false
        }
        delegate: dlg
    }
    Component{
        id:dlg
            Image{
                id:image
                width: gridView.cellWidth-10
                mipmap : true
                cache:true
                asynchronous:true
                source:"file:///"+filePath
                fillMode: Image.PreserveAspectFit
                Rectangle{
                    id:imageShadowRect
                    anchors.fill: parent
                    visible:false
                }
                RectangularGlow {
                    id: effect
                    z:-1
                    anchors.fill: imageShadowRect
                    glowRadius: width/40
                    spread: 0
                    color: "black"
                    cornerRadius: imageShadowRect.radius + glowRadius
                    //opacity: 0
                    visible: image.status === Image.Ready
                }

            states: [
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
                    print("picture clicked")
                }
            }
            }
    }

}

