import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import Qt.labs.folderlistmodel 2.1

Popup {
    id:coverflow
    property ListModel model
    property int itemCount: Math.floor(width/200)
    property string pictureDirectory: undefined
    signal imageClicked(var image)
    background:Rectangle{
        //color: "transparent"
       // opacity: 0.5
        RectangularGlow {
            id: backgroundEffect
            z:-1
            anchors.fill: parent
            glowRadius: width/40
            spread: 0
            color: "#80000000"
            cornerRadius: parent.radius + glowRadius
        }
    }
    //color: "black"
    PathView{
        id:pathView
        path:coverFlowPath
        pathItemCount: coverflow.itemCount
        anchors.fill: parent
        preferredHighlightBegin: 0.48
        preferredHighlightEnd: 0.52
        model:FolderListModel{
            folder:coverflow.pictureDirectory// "file:/C:\Work\tempProjects\VisualizerQml\images"
            nameFilters: [ "*.png", "*.jpg" ]
            showDirs : false
        }
        delegate: Item {
            id:delegateItem
            width: pathView.width/itemCount
            height: pathView.height
            z:PathView.iconZ
            scale:PathView.iconScale

            Image{
                id:image
                //width: delegateItem.width
                height: delegateItem.height*0.7
                mipmap : true
                cache:true
                asynchronous:true
                antialiasing:true
                source: "file:///"+filePath
                fillMode: Image.PreserveAspectFit
                property bool isPopup: false
                Rectangle{
                    id:imageShadowRect
                    anchors.centerIn: parent
                    width: parent.paintedWidth
                    height: parent.paintedHeight
                    visible:false
                }
                RectangularGlow {
                    id: effect
                    z:-1
                    anchors.fill: imageShadowRect
                    glowRadius: width/40
                    spread: 0
                    color: image.isPopup?"#FFE91E63":"black"
                    cornerRadius: imageShadowRect.radius + glowRadius
                    //opacity: 0
                    visible: image.status === Image.Ready
                }
                MouseArea {
                    id:mouse
                    anchors.fill: parent
                    hoverEnabled:true
                    onClicked:{
                        imageClicked(image)
                    }
                }
            }
            ShaderEffect {
                anchors.top: image.bottom
                anchors.bottom: delegateItem.bottom
                width: image.paintedWidth
                //height: delegateItem.height-image.height
                anchors.left: image.left
                property variant source: image
                property size sourceSize: Qt.size(0.5 / image.paintedWidth, 0.5 / image.paintedHeight);
                fragmentShader:
                        "varying highp vec2 qt_TexCoord0;
                        uniform lowp sampler2D source;
                        uniform lowp vec2 sourceSize;
                        uniform lowp float qt_Opacity;
                        void main() {
                            lowp vec2 tc = qt_TexCoord0 * vec2(1, -1) + vec2(0, 1);
                            lowp vec4 col = 0.25 * (texture2D(source, tc + sourceSize) + texture2D(source, tc- sourceSize)
                            + texture2D(source, tc + sourceSize * vec2(1, -1))
                            + texture2D(source, tc + sourceSize * vec2(-1, 1)));
                            gl_FragColor = col * qt_Opacity * (1.0 - qt_TexCoord0.y) * 0.2;
                        }"
            }

            transform: Rotation{
                origin.x:image.x+image.paintedWidth*0.25
                origin.y:image.y+image.paintedHeight*0.25
                axis{x:0;y:1;z:0}
                angle: delegateItem.PathView.iconAngle
            }
            states: [
            State {
                         name: "mouseEnter"
                         when: mouse.containsMouse
                         PropertyChanges { target: delegateItem; scale: PathView.isCurrentItem?1.1:1.0}
                    }
            ]

            transitions: Transition {
                NumberAnimation { easing.type: Easing.OutCubic; properties: "scale"; duration: 300 }
            }
        }
    }

    Path{
        id:coverFlowPath
        startX: 0
        startY: coverflow.height*0.6
        PathAttribute{name:"iconZ";value: 0}
        PathAttribute{name:"iconAngle";value: 90}
        PathAttribute{name:"iconScale";value: 0.6}
        PathLine{x:coverflow.width/2;y:coverflow.height*0.6}

        PathAttribute{name:"iconZ";value: 100}
        PathAttribute{name:"iconAngle";value: 0}
        PathAttribute{name:"iconScale";value: 1.0}

        PathLine{x:coverflow.width;y:coverflow.height*0.6}
        PathAttribute{name:"iconZ";value: 0}
        PathAttribute{name:"iconAngle";value: -90}
        PathAttribute{name:"iconScale";value: 0.6}
        PathPercent{value:1.0}

    }

}
