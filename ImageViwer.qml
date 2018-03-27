import QtQuick 2.0
import QtQuick.Controls 2.2
Rectangle{
    id:popImage
   // background: Rectangle{
        color: "black"
   // }
    property var sourceImage: popImage
    property bool isPopup:false
    property real animationTime: 200
    property var backgroundItem: undefined
    Image{
        id:mainImage
        anchors.fill: parent
        cache:true
        asynchronous:true
        antialiasing:true
        fillMode: Image.PreserveAspectFit
    }
    SequentialAnimation {
       id:popupAnimation
       PropertyAction{target:mainImage;property: "source";value: popImage.sourceImage.source;}
       PropertyAction{target:popImage;property: "visible";value: true;}
       PropertyAction{target:popImage;property: "opacity";value: 0;}
       PropertyAction{target:popImage;property: "x";value: sourceImagePoint().x;}
       PropertyAction{target:popImage;property: "y";value: sourceImagePoint().y;}
       PropertyAction{target:popImage;property: "width";value: popImage.sourceImage.width;}
       PropertyAction{target:popImage;property: "height";value: popImage.sourceImage.height;}

       ParallelAnimation {
            NumberAnimation { target:popImage;property: "opacity"; to: 1; duration: popImage.animationTime; easing.type: Easing.OutQuad }
            NumberAnimation { target:popImage;property: "x"; to: 0; duration: popImage.animationTime; easing.type: Easing.OutQuad }
            NumberAnimation { target:popImage;property: "y"; to: 0; duration: popImage.animationTime; easing.type: Easing.OutQuad }
            NumberAnimation { target:popImage;property: "width"; to: root.width; duration: popImage.animationTime; easing.type: Easing.OutQuad }
            NumberAnimation { target:popImage;property: "height"; to: root.height; duration: popImage.animationTime; easing.type: Easing.OutQuad }
            NumberAnimation { target:popImage.backgroundItem;property: "scale"; to: popImage.backgroundItem.scale+0.03; duration: popImage.animationTime; easing.type: Easing.Linear }
        }
       PropertyAction{target:popImage;property: "isPopup";value: true;}
       PropertyAction{target:popImage.sourceImage;property: "isPopup";value: true;}
    }
    SequentialAnimation {
       id:popdownAnimation
      ParallelAnimation {
            NumberAnimation { target:popImage;property: "opacity"; to: 0; duration: popImage.animationTime; easing.type: Easing.InQuad }
            NumberAnimation { target:popImage;property: "x"; to: sourceImagePoint().x; duration: popImage.animationTime; easing.type: Easing.InQuad }
            NumberAnimation { target:popImage;property: "y"; to: sourceImagePoint().y; duration: popImage.animationTime; easing.type: Easing.InQuad }
            NumberAnimation { target:popImage;property: "width"; to: popImage.sourceImage.width; duration: popImage.animationTime; easing.type: Easing.InQuad }
            NumberAnimation { target:popImage;property: "height"; to: popImage.sourceImage.height; duration: popImage.animationTime; easing.type: Easing.InQuad }
            NumberAnimation { target:popImage.backgroundItem;property: "scale"; to: popImage.backgroundItem.scale-0.03; duration: popImage.animationTime; easing.type: Easing.Linear }
      }
       PropertyAction{target:popImage;property: "visible";value: false;}
       PropertyAction{target:mainImage;property: "source";value: undefined;}
       PropertyAction{target:popImage;property: "isPopup";value: false;}
       PropertyAction{target:popImage.sourceImage;property: "isPopup";value: false;}
    }
        function popup()
        {
            popupAnimation.start()
        }
        function popdown()
        {
            popdownAnimation.start()
        }
        function setSourceImage(image)
        {
            if(popupAnimation.running)
            {
                popupAnimation.complete()
            }
            if(popdownAnimation.running)
            {
                popdownAnimation.complete()
            }

            if(image===sourceImage)
            {
                if(isPopup)popdown()
                else popup()
            }
            else
            {
                if(sourceImage)sourceImage.isPopup=false
                sourceImage=image
                if(image)
                    popup()
            }
        }
        function sourceImagePoint()
        {
            return sourceImage.mapToItem(root,sourceImage.x,sourceImage.y)
        }
}

