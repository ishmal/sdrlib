
import QtQuick 2.0
import Sdr 1.0

Rectangle {
    property color buttonColor: "lightblue"
    property color onHoverColor: "gold"
    property color borderColor: "white"

    width: 360
    height: 360
    Text {
        anchors.centerIn: parent
        text: "Hello World"
    }
    Waterfall {
        objectName: "waterfall"
    }
    Rectangle {
        x: 0
        y: 0
        width: 60
        height: 20
        Text {
            anchors.centerIn: parent
            text: "Start"
        }
        MouseArea {
             anchors.fill: parent
             hoverEnabled: true
             onEntered: parent.border.color = onHoverColor
             onExited:  parent.border.color = borderColor
             onClicked: sdr.start()
        }
    }
    Rectangle {
        x: 100
        y: 0
        width: 60
        height: 20
        Text {
            anchors.centerIn: parent
            text: "Stop"
        }
        MouseArea {
             anchors.fill: parent
             hoverEnabled: true
             onEntered: parent.border.color = onHoverColor
             onExited:  parent.border.color = borderColor
             onClicked: sdr.stop()
        }
    }
}
