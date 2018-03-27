import QtQuick 2.7
import QtQuick.Controls 2.2
import ophelia.qml.controls 1.0

Rectangle{
    width: 800
    height: 600
    //color:"#00B000"
    ListModel {
        id:videoList

          ListElement {
              name: "Brown"
              url:"rtmp://live.hkstv.hk.lxdns.com/live/hks"
          }
          ListElement {
              name: "John Brown"
              url: "C:\\Users\\liuji\\Videos\\Adele - Rolling in the Deep (Piano_Cello Cover) - ThePianoGuys.mp4"
          }
          ListElement {
              name: "Bill Smyth"
              url: "video=Microsoft Camera Front"
          }
          ListElement {
              name: "Sam Wise"
              url: "C:\\Users\\liuji\\Videos\\All of Me (Jon Schmidt) - ThePianoGuys.mp4"
          }

          ListElement {
              name: "Sam Wise2"
              url: "C:\\Users\\liuji\\Videos\\dahufa.mkv"
          }
          ListElement {
              name: "Sam Wise3"
              url: "spyderman.rmvb"
          }
          ListElement {
              name: "Sam Wise"
              url: "rtmp://live.hkstv.hk.lxdns.com/live/hks"
          }

          ListElement {
              name: "Sam Wise2"
              url: "rtmp://live.hkstv.hk.lxdns.com/live/hks"
          }
          ListElement {
              name: "Sam Wise3"
              url: "C:\\Users\\liuji\\Music\\2CELLOS - Back In Black (Live ADDC Cover).mp3"
          }
      }


    GridView{
        id:gridView
        anchors.fill:parent
        model: videoList
        delegate:videoDelegate
        cellWidth: Math.floor(width/count)
        cellHeight: height
        Component.onCompleted:{
            //camera.play();
            console.log("cellWidth:",cellWidth,"cellHeight:",cellHeight)
        }

    }
    Component{
                id:videoDelegate
                Rectangle{
                    //anchors.fill:parent
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                    color:"#000000"

                    Camera{
                        id:camera
                        anchors.fill:parent
                        source: url
                        Button{
                            id:btnPlay
                            anchors.right:camera.right
                            //anchors.centerIn: parent
                            anchors.verticalCenter: camera.verticalCenter
                            text:parent.isPlaying?qsTr("Stop"):qsTr("Play")
                            onClicked: {
                                if(parent.isPlaying)
                                {
                                    camera.stop()
                                    text="Play"
                                }
                                else{
                                    parent.play()
                                    text="Stop"
                                    //btnPlay.visible=false;
                                }
                            }
                        }
                    }

                }
            }


}
