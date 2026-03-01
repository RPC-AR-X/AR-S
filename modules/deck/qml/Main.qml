import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import "."
import "components"

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: "Deck"
    color: Theme.polarNight2

    property real uiScale: 1.0
    Shortcut { sequence: "Ctrl+="; onActivated: window.uiScale = Math.min(window.uiScale + 0.1, 2.0) }
    Shortcut { sequence: "Ctrl+-"; onActivated: window.uiScale = Math.max(window.uiScale - 0.1, 0.5) }
    Shortcut { sequence: "Ctrl+0"; onActivated: window.uiScale = 1.0 }

    property bool isGithubAuthenticated: false

    AuthPopup {
        id: githubAuthPopup
    }

    Connections {
        target: AuthManager 

        function onDeviceAuthReady(url, code) {
            githubAuthPopup.authUrl = url
            githubAuthPopup.userCode = code
            githubAuthPopup.open()
        }

        function onTokenReceived(token) {
            githubAuthPopup.close()
            console.log("Token received, UI updated.")
            window.isGithubAuthenticated = true
        }
        
        function onTokenErrorReceived(error) {
             console.error("Auth error: " + error)
        }
    }

    Item {
        anchors.centerIn: parent
        width: parent.width / window.uiScale
        height: parent.height / window.uiScale
        scale: window.uiScale
        transformOrigin: Item.Center

        SplitView {
            anchors.fill: parent
            handle: Rectangle { implicitWidth: 1; color: Theme.polarNight1 }

            Rectangle {
                SplitView.preferredWidth: 250
                SplitView.minimumWidth: 200
                SplitView.maximumWidth: 350
                color: Theme.polarNight1

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true; height: 80; color: "transparent"
                        Text {
                            text: "Deck"
                            color: Theme.snowStorm3
                            font.pixelSize: 20; font.bold: true
                            anchors.centerIn: parent
                        }
                    }

                    ListView {
                        id: sidebarList
                        Layout.fillWidth: true; Layout.fillHeight: true
                        clip: true
                        model: ListModel {
                            ListElement { name: "Dashboard";}
                            ListElement { name: "Sonar Module"; }
                            ListElement { name: "Magfield Module"; }
                            ListElement { name: "Interlink Module"; }
                        }

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 48
                            color: ListView.isCurrentItem ? Theme.polarNight3 : "transparent"

                            Rectangle {
                                width: 4; height: parent.height
                                color: Theme.frost2
                                visible: parent.ListView.isCurrentItem
                            }

                            Text {
                                text: model.name
                                color: parent.ListView.isCurrentItem ? Theme.snowStorm2 : Theme.snowStorm1
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left; anchors.leftMargin: 24
                            }
                            MouseArea { anchors.fill: parent; onClicked: sidebarList.currentIndex = index }
                        }
                    }
                }
            }

            Rectangle {
                SplitView.fillWidth: true
                color: Theme.polarNight2

                StackLayout {
                    anchors.fill: parent
                    currentIndex: sidebarList.currentIndex

                    Item { Text { text: "Dashboard Placeholder"; color: Theme.snowStorm1; anchors.centerIn: parent } }

                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.defaultMargin
                            spacing: 16
                            visible: window.isGithubAuthenticated

                            RowLayout {
                                Layout.fillWidth: true
                                Text {
                                    text: "Build Pipelines"
                                    color: Theme.snowStorm3
                                    font.pixelSize: 24; font.bold: true
                                    Layout.fillWidth: true
                                }

                                Rectangle {
                                    color: "transparent"
                                    border.color: Theme.auroraGreen
                                    border.width: 1
                                    radius: 6
                                    implicitWidth: 100; implicitHeight: 30
                                    
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 6
                                        Text { text: "✓"; color: Theme.auroraGreen; font.bold: true }
                                        Text { text: "Connected"; color: Theme.auroraGreen; font.pixelSize: 12; font.bold: true }
                                    }
                                }

                                Button {
                                    id: refreshBtn
                                    text: "⟳ Refresh"
                                    flat: true
                                    
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.pressed ? Theme.polarNight1 : Theme.frost2
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: parent.pressed ? Theme.frost2: (parent.hovered ? Theme.polarNight3 : "transparent")
                                        radius: 6
                                        border.width: 1
                                        border.color: parent.hovered ? Theme.frost2 : Theme.polarNight4
                                    }
                                    onClicked: SonarViewModel.fetchPipelineData()
                                }
                            }

                            ListView {
                                id: pipelineListView
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true
                                spacing: 8
                                model: SonarViewModel

                                add: Transition {
                                    NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 300 }
                                    NumberAnimation { property: "y"; from: 20; to: 0; duration: 400; easing.type: Easing.OutQuart }
                                }

                                delegate: Rectangle {
                                    id: cardDelegate
                                    width: pipelineListView.width
                                    height: 60
                                    radius: Theme.defaultRadius
                                    color: mouseArea.containsMouse ? Theme.polarNight4 : Theme.polarNight3
                                    Behavior on color { ColorAnimation { duration: 150 } }

                                    MouseArea {
                                        id: mouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: console.log("Clicked pipeline " + model.id)
                                    }

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 16
                                        anchors.rightMargin: 16
                                        spacing: 16

                                        Rectangle {
                                            width: 24; height: 24
                                            radius: 12
                                            color: model.conclusion === "success" ? Theme.auroraGreen :
                                            model.conclusion === "failure" ? Theme.auroraRed : Theme.auroraYellow

                                            Shape {
                                                anchors.centerIn: parent
                                                width: 10; height: 10
                                                visible: model.conclusion === "success"
                                                ShapePath {
                                                    strokeWidth: 2; strokeColor: Theme.iconOnLight
                                                    fillColor: "transparent"; capStyle: ShapePath.RoundCap
                                                    startX: 1; startY: 5; PathLine { x: 4; y: 8 } PathLine { x: 9; y: 1 }
                                                }
                                            }
                                            Text {
                                                anchors.centerIn: parent
                                                text: "✕"; color: Theme.iconOnLight; font.pixelSize: 12; font.bold: true
                                                visible: model.conclusion === "failure"
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: model.name ? model.name : "Unknown"
                                                color: Theme.snowStorm2
                                                font.pixelSize: 14; font.bold: true
                                            }
                                            Text {
                                                text: "ID: " + model.id + " • " + model.status
                                                color: Theme.snowStorm1
                                                font.pixelSize: 11; opacity: 0.6
                                            }
                                        }
                                        
                                        Button {
                                            text: "Logs"
                                            flat: true
                                            Layout.preferredHeight: 28
                                            contentItem: Text {
                                                text: parent.text; color: parent.hovered ? Theme.snowStorm3 : Theme.frost3
                                                font.pixelSize: 11; font.bold: true
                                                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                                            }
                                            background: Rectangle {
                                                color: parent.pressed ? Theme.polarNight1: (parent.hovered ? Theme.polarNight4 : "transparent")
                                                radius: 4; border.width: 1; border.color: Theme.polarNight4
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 32
                            visible: !window.isGithubAuthenticated

                            ColumnLayout {
                                Layout.alignment: Qt.AlignHCenter
                                spacing: 8
                                Text {
                                    text: "Sonar Module"
                                    font.pixelSize: 32; font.bold: true
                                    color: Theme.snowStorm1
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Text {
                                    text: "CI/CD Pipeline Monitor"
                                    font.pixelSize: 16
                                    color: Theme.snowStorm2
                                    Layout.alignment: Qt.AlignHCenter
                                    opacity: 0.8
                                }
                            }

                            Button {
                                id: githubSignInBtn
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredWidth: 220
                                Layout.preferredHeight: 48

                                contentItem: RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 12

                                    Shape {
                                        Layout.preferredWidth: 24
                                        Layout.preferredHeight: 24
                                        Layout.leftMargin: 6
                                        ShapePath {
                                            strokeWidth: 0
                                            fillColor: githubSignInBtn.down ? Theme.polarNight1 : Theme.snowStorm1 
                                            PathSvg { path: "M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" }
                                        }
                                    }

                                    Text {
                                        text: "Sign in with GitHub"
                                        color: githubSignInBtn.down ? Theme.polarNight1 : Theme.snowStorm1 
                                        font.bold: true
                                        font.pixelSize: 16
                                    }
                                }

                                background: Rectangle {
                                    color: parent.down ? Theme.snowStorm1 : "transparent"
                                    border.color: Theme.snowStorm1
                                    border.width: 2
                                    radius: 8
                                    Behavior on color { ColorAnimation { duration: 100 } }
                                }

                                onClicked: {
                                    console.log("Starting OAuth flow...")
                                    AuthManager.startAuth()
                                }
                            }
                        }
                    }

                    Item { Text { text: "Magfield Module"; color: Theme.snowStorm1; anchors.centerIn: parent } }
                    Item { Text { text: "Interlink Module"; color: Theme.snowStorm1; anchors.centerIn: parent } }
                }
            }
        }
    }
}