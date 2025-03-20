import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: rootRect
    anchors.fill: parent
    color: "#f5f5f5"
    visible: true

    // Link to C++ properties
    property int timerValue: app.timerValue
    property string usbInfo: app.usbList
    // If you use app.buttonText in C++ for your Start/Stop button, it appears here
    // property string buttonText: app.buttonText  // optional if you prefer to store locally

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Timer Display
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#e0e0e0"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Elapsed Time: " + timerValue + " seconds"
                font.pixelSize: 20
                font.bold: true
                color: "#333333"
            }
        }

        // Start/Stop Button
        Button {
            id: startStopButton
            Layout.alignment: Qt.AlignHCenter
            // Uses the button text from C++: app.buttonText
            text: app.buttonText
            onClicked: app.onStartStopClicked()

            contentItem: Text {
                text: startStopButton.text
                font.pixelSize: 16
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: app.buttonText === "Start" ? "#4CAF50" : "#F44336"
                radius: 5
                implicitWidth: 150
                implicitHeight: 40
            }

            Layout.preferredWidth: 150
            Layout.preferredHeight: 40
        }

        // USB Devices Section
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "USB Devices"
            font.bold: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Button {
                    text: "List USB Devices"
                    onClicked: app.onListUSBClicked()
                    Layout.fillWidth: true

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        color: "#2196F3"
                        radius: 5
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TextArea {
                        id: usbOutput
                        text: usbInfo
                        readOnly: true
                        font.family: "Monospace"
                        font.pixelSize: 12
                        wrapMode: Text.WrapAnywhere
                        padding: 10

                        background: Rectangle {
                            color: "white"
                            border.color: "#cccccc"
                            radius: 5
                        }
                    }
                }
            }
        }

        // Control Buttons Row
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "Exit"
                onClicked: app.onExitClicked()
                Layout.preferredWidth: 100

                contentItem: Text {
                    text: parent.text
                    color: "white"
                }

                background: Rectangle {
                    color: "#607D8B"
                    radius: 5
                }
            }

            Button {
                text: "Cleanup"
                onClicked: app.onCleanupClicked()
                Layout.preferredWidth: 100

                contentItem: Text {
                    text: parent.text
                    color: "white"
                }

                background: Rectangle {
                    color: "#9E9E9E"
                    radius: 5
                }
            }
        }
    }
}
