import QtQuick.Controls 2.2
import QtQuick 2.7

    Button{
                id:btn
                //implicitHeight:ListView.view.height
                //implicitWidth: implicitHeight
                hoverEnabled:true
                property string backgroundUrl:""
                signal buttonClicked(var sender)
                background: Image {
                    id:backgroundImage
                    anchors.fill:btn
                    fillMode: Image.PreserveAspectFit
                    source: backgroundUrl
                    Rectangle{
                        id:rect
                        anchors.fill: parent
                        color: "transparent"
                        radius: width/20<4?4:width/20
                        border.color: "#0070c0"
                        border.width: btn.highlighted?4:0
                    }
                   }
                states: [
                 State {
                      name: "hovered"
                        when:btn.hovered
                        PropertyChanges {
                            target: backgroundImage
                            scale: 0.9
                        }
                 }


                ]
                 Transition {
                      NumberAnimation { target:backgroundImage;properties: "scale";easing.type: Easing.OutCubic; duration: 300 }
                 }
//                 onClicked: {
//                    print("Btn :",btn,"clicked")
//                     //Button.clicked()
//                 }
            }

