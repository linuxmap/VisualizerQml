import QtQuick 2.7
import QtQuick.Controls 2.2
//import QtLocation 5.3
//import QtGraphicalEffects 1.0
//import "common.js" as Common
Item{
    id:root
    //anchors.margins: 10
    width: 100
    height: 100
    property bool shadow: true
    property  var anotaionDialog: 0
    property  var picturesDialog: 0
    property var adjustDialog:0
    property  var settingsDialog: 0

    property var dialogs:[settingsDialog,picturesDialog,anotaionDialog]
    property int moveButtonId:0
    property int anotationButtonId:1
    property int eraserButtonId:2
    property int undoButtonId:3
    property int catchImageButtonId:4
    property int pictureGallaryButtonId:5
    property int adjustButtonId:6
    property int settingsButtonId:7
    property var buttonsArray:[]
    //opacity: 0.5
    property alias buttonCount: view.count
    property bool anotationEnable: false
    property var anotationlayer: 0
    ListModel {
        id:buttons
         ListElement {
             //elementId:moveButtonId
             name: "Move"
             url: "qrc:/images/move.png"
             hasDialog:"yes"
         }
         ListElement {
             //elementId:anotationButtonId
             name: "Anotation"
             url: "/images/anotation.png"
             hasDialog:"yes"
         }
         ListElement {
             //elementId:eraserButtonId
             name: "Eraser"
             url: "/images/eraser.png"
             hasDialog:"yes"
         }
         ListElement {
             //elementId:undoButtonId
             name: "Undo"
             url: "/images/undo.png"
             hasDialog:"no"
         }
         ListElement {
             //elementId:catchImageButtonId
             name: "Catch image"
             url: "/images/takepicture.png"
             hasDialog:"yes"
         }
         ListElement {
             //elementId:pictureGallaryButtonId
             name: "Picture gallary"
             url: "/images/photofolder.png"
             hasDialog:"yes"

         }
         ListElement {
             //elementId:adjustButtonId
             name: "Adjust"
             url: "/images/adjust.png"
             hasDialog:"yes"
         }
         ListElement {
             //elementId:settingsButtonId
             name: "Settings"
             url: "/images/settings.png"
             hasDialog:"no"
         }

     }
    ListView{
        id:view
        anchors.fill:parent
        displayMarginBeginning:10
        orientation:Qt.Horizontal
        model:buttons
        delegate: buttonDelegate
        Component.onCompleted: {
                  anotationEnable = Qt.binding(function() {
                      return  buttonsArray[anotationButtonId].highlighted
                  });
                buttonsArray[moveButtonId].highlighted=true
              }
    }
    Component{
        id:buttonDelegate
        Button{
                    id:btn
                    width: view.width/view.count
                    height: view.height
                    hoverEnabled:true
                    contentItem: Image {
                                fillMode: Image.PreserveAspectFit
                                source: url
                                mipmap : true
                                cache:true
                                asynchronous:true
                                antialiasing:true
                               }
                    onClicked: {
                        switch(index)
                        {
                        case moveButtonId: //Move button
                            highlighted=true
                            root.buttonsArray[anotationButtonId].highlighted=false
                            root.buttonsArray[eraserButtonId].highlighted=false
                            break;
                        case anotationButtonId://Anotation button
                            if(!highlighted){
                            highlighted=true
                            root.buttonsArray[moveButtonId].highlighted=false
                            root.buttonsArray[eraserButtonId].highlighted=false
                            }
                            else{
                                anotaionDialog.visible=!anotaionDialog.visible
                            }
                            break;
                        case eraserButtonId://Eraser button
                            highlighted=true
                            root.buttonsArray[moveButtonId].highlighted=false
                            root.buttonsArray[anotationButtonId].highlighted=false
                            break;
                        case undoButtonId:
                            anotationlayer.undo()
                            break;
                        case catchImageButtonId:
                            camera.catchImage();
                            break;
                         case pictureGallaryButtonId:
                             picturesDialog.visible?picturesDialog.close():picturesDialog.open()
                             break
                         case adjustButtonId:
                             adjustDialog.visible?adjustDialog.close():adjustDialog.open()
                             break
                         case settingsButtonId:
                             settingsDialog.visible?settingsDialog.close():settingsDialog.open()
                             break
                        default:break;
                        }
                    }
                    states: [
                        State {
                            name: "hovered"
                            when:btn.hovered
                        },
                        State{
                            name: "nohovered"
                            when:btn.hovered===false
                        }
                    ]
                    Component.onCompleted:
                    {
                        root.buttonsArray.push(btn)
                    }
                }

    }

}







