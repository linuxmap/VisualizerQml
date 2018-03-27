import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

Popup {
    id:root
    property string fontFamily: "Verdana"
    property var anotationlayer:0
    property real penWidth: penWidthSlider.value
    ColumnLayout{
        id:mainLayout
        clip: false
        anchors.fill: parent
        anchors.margins: parent.width/20
        spacing: height/20
        GridLayout {
              id: grid
              width: parent.width
              Layout.fillHeight: false
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
              Layout.preferredHeight:parent.height*0.6
              columns: 4
              rows:4
              property real radius: cellWidth/10
              property real cellWidth: width/5
              property real cellHeight: height/5

              property var model: ["#FFC00000","#FFFF0000","#FFFFC000","#FFFFFF00",
                                    "#FF92D050","#FF00B050","#FF00B0F0","#FF0070C0",
                                    "#FF002060","#FF7030A0","#FF00B0F0","#FF000000",
                                    "#FF002060","#FF7030A0","#FFFFC000","#FF000000",
                                ]
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[0] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[1] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[2] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[3] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[4] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[5] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[6] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[7] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[8] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[9] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[10] }
              ColorCell{/*width:grid.cellWidth;height:grid.cellHeight;*/radius: grid.radius; Layout.fillHeight: true; Layout.fillWidth: true; color: grid.model[11] }
          }
        RowLayout{
            id:lineWidth
            spacing: 0
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //Layout.alignment: Qt.AlignCenter
            Text{
                text:qsTr("Line Width:")
                fontSizeMode: Text.FixedSize
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.fillHeight: true
                Layout.fillWidth: false
                font.family: root.fontFamily
                }
                Slider {
                    id:penWidthSlider
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    from:1.0
                    to:30
                    value: 10
                    ToolTip{
                        x: (parent.width-width)/2
                        y:parent.height/2-height*1.2
                        visible: penWidthSlider.pressed
                        text:Math.floor(penWidthSlider.value)
                    }
                }
        }
    }
}
