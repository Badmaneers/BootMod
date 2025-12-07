import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: aboutDialog
    title: "About BootMod"
    modal: true
    anchors.centerIn: parent
    width: 500
    height: 680
    
    property color bgColor: "#1e1e1e"
    property color surfaceColor: "#252526"
    property color borderColor: "#3e3e42"
    property color textColor: "#cccccc"
    property color textSecondaryColor: "#858585"
    property color accentColor: "#007acc"
    property color accentHoverColor: "#005a9e"
    property int radius: 6
    
    background: Rectangle {
        color: surfaceColor
        border.color: borderColor
        border.width: 1
        radius: radius
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16
        
        // Logo and Title
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
            
            Image {
                source: "qrc:/BootMod/res/bootmod_logo.svg"
                sourceSize.width: 56
                sourceSize.height: 56
                smooth: true
            }
            
            ColumnLayout {
                spacing: 4
                
                Text {
                    text: "BootMod"
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                }
                
                Text {
                    text: "Version " + appVersion
                    color: textSecondaryColor
                    font.pixelSize: 12
                }
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }
        
        Text {
            text: "A powerful GUI tool for unpacking, editing, and repacking boot logo files. Supports both MediaTek logo.bin and Qualcomm Snapdragon splash.img formats. Built with Qt6 and modern C++."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: textColor
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
        }
        
        Item { Layout.preferredHeight: 4 }
        
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Image {
                source: "qrc:/BootMod/res/icons/sparkles.svg"
                width: 16
                height: 16
                sourceSize: Qt.size(16, 16)
                Layout.alignment: Qt.AlignVCenter
            }
            
            Text {
                text: "Features:"
                font.bold: true
                font.pixelSize: 14
                color: textColor
                Layout.alignment: Qt.AlignVCenter
            }
        }
        
        GridLayout {
            columns: 2
            columnSpacing: 30
            rowSpacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Text {
                text: "• MediaTek logo.bin support"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Qualcomm splash.img support"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Extract logo images"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Replace logos easily"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Project-based workflow"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Native file dialogs"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Live thumbnail updates"
                color: textSecondaryColor
                font.pixelSize: 11
            }
            Text {
                text: "• Drag & drop support"
                color: textSecondaryColor
                font.pixelSize: 11
            }
        }
        
        Item { Layout.fillHeight: true }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }
        
        // Developer Info
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            
            RowLayout {
                spacing: 8
                Layout.alignment: Qt.AlignHCenter
                
                Image {
                    source: "qrc:/BootMod/res/icons/user.svg"
                    width: 16
                    height: 16
                    sourceSize: Qt.size(16, 16)
                    Layout.alignment: Qt.AlignVCenter
                }
                
                Text {
                    text: "Developer"
                    font.bold: true
                    font.pixelSize: 13
                    color: textColor
                    Layout.alignment: Qt.AlignVCenter
                }
            }
            
            // GitHub Link
            RowLayout {
                spacing: 8
                Layout.alignment: Qt.AlignHCenter
                
                Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: "#24292e"
                    
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/BootMod/res/icons/link.svg"
                        width: 14
                        height: 14
                        sourceSize: Qt.size(14, 14)
                    }
                }
                
                Text {
                    text: "GitHub: Badmaneers"
                    color: textColor
                    font.pixelSize: 12
                }
                
                Button {
                    text: "Visit"
                    flat: true
                    Layout.preferredHeight: 28
                    onClicked: Qt.openUrlExternally("https://github.com/Badmaneers")
                    
                    background: Rectangle {
                        color: parent.hovered ? accentHoverColor : accentColor
                        radius: 4
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 11
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            
            // Telegram Link
            RowLayout {
                spacing: 8
                Layout.alignment: Qt.AlignHCenter
                
                Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: "#0088cc"
                    
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/BootMod/res/icons/telegram.svg"
                        width: 14
                        height: 14
                        sourceSize: Qt.size(14, 14)
                    }
                }
                
                Text {
                    text: "Telegram: @DumbDragon"
                    color: textColor
                    font.pixelSize: 12
                }
                
                Button {
                    text: "Open"
                    flat: true
                    Layout.preferredHeight: 28
                    onClicked: Qt.openUrlExternally("https://t.me/DumbDragon")
                    
                    background: Rectangle {
                        color: parent.hovered ? accentHoverColor : accentColor
                        radius: 4
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 11
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
        
        Item { Layout.preferredHeight: 4 }
        
        RowLayout {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter
            
            Text {
                text: "Made with"
                color: textSecondaryColor
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }
            
            Image {
                source: "qrc:/BootMod/res/icons/heart.svg"
                width: 14
                height: 14
                sourceSize: Qt.size(14, 14)
                Layout.alignment: Qt.AlignVCenter
            }
            
            Text {
                text: "by Badmaneers"
                color: textSecondaryColor
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }
        }
        
        Text {
            text: "© 2025 BootMod · MIT License"
            color: textSecondaryColor
            font.pixelSize: 10
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
