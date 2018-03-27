import QtQuick 2.7
import QtQuick.Controls 2.2
//import QtGraphicalEffects 1.0
//import "common.js" as Common
Rectangle{
    width: 600
    height: 100
    //id:root
    ListModel {
        id:buttons
         ListElement {
             name: "Bill Smith"
             url: "/images/Adobe Photoshop CS5.png"
             hasDialog:"yes"

         }
         ListElement {
             name: "John Brown"
             url: "/images/Game Center2.png"
             hasDialog:"yes"

         }
         ListElement {
             name: "Sam Wise"
             url: "/images/Icon Resourcer.png"
             hasDialog:"yes"

         }
         ListElement {
             name: "Sam Wise"
             url: "/images/Icon Resourcer.png"
             hasDialog:"no"
         }

     }
    ListView{
        id:view
        anchors.fill:parent
        //anchors.margins : 10
        //implicitWidth:60
        //implicitHeight: 60
        orientation:Qt.Horizontal
        //spacing: width/100
        model:buttons
        delegate: buttonDelegate
    }
    Component{
        id:buttonDelegate
        Button{
            id:btn
            width: 60//view.width
            height: view.height
            hoverEnabled:true
                    //background:
            contentItem: Image {
                         fillMode: Image.PreserveAspectFit
                         source: url
                         }
//                    onClicked: {
//                        switch(index)
//                        {
//                        case 3:
//                            camera.catchImage();
//                            return;
//                        default:break;
//                        }
//                        root.dialogs[index].visible?root.dialogs[index].close():root.dialogs[index].open()

//                    }
//                    states: [
//                        State {
//                            name: "hovered"
//                            when:btn.hovered
//                        },
//                        State{
//                            name: "nohovered"
//                            when:btn.hovered===false
//                        }
//                    ]
//                    transitions: [
//                              Transition {
//                                from:"nohovered"
//                                to:"hovered"
//                                 NumberAnimation { target:btn;properties: "y";to:-20 ;easing.type: Easing.InOutQuad; duration: 200 }
//                              },
//                              Transition {
//                                from:"hovered"
//                                to: "nohovered"
//                                NumberAnimation { target:btn;properties: "y";to:0 ;easing.type: Easing.InOutQuad; duration: 200 }

//                              }
//                          ]
                }
    }

}
