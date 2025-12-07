import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import QtQuick.Dialogs
import BootMod 1.0

ApplicationWindow {
    id: root
    width: 1200
    height: 800
    visible: true
    title: logoFile.isLoaded ? "BootMod - " + logoFile.filePath : "BootMod - Boot Logo Manager"
    
    // Theme properties
    readonly property bool isDark: true
    readonly property color bgColor: isDark ? "#1e1e1e" : "#f5f5f5"
    readonly property color surfaceColor: isDark ? "#252526" : "#ffffff"
    readonly property color borderColor: isDark ? "#3e3e42" : "#d0d0d0"
    readonly property color textColor: isDark ? "#cccccc" : "#333333"
    readonly property color textSecondaryColor: isDark ? "#858585" : "#666666"
    readonly property color accentColor: "#007acc"
    readonly property color accentHoverColor: "#1f86cc"
    readonly property color successColor: "#4ec9b0"
    readonly property color errorColor: "#f48771"
    
    readonly property int spacing: 16
    readonly property int radius: 6
    
    // Helper function to convert URL to local path
    function urlToPath(urlString) {
        var path = urlString.toString()
        if (path.startsWith("file://")) {
            path = path.substring(7)
        }
        return path
    }
    
    // Backend
    LogoFile {
        id: logoFile
        onOperationCompleted: (message) => statusText.showSuccess(message)
        onErrorOccurred: (message) => statusText.showError(message)
    }
    
    LogoListModel {
        id: logoModel
        logoFile: logoFile
    }
    
    // File Dialogs (using native system dialogs)
    FileDialog {
        id: openDialog
        title: "Open Boot Logo File"
        nameFilters: ["Boot Logo files (*.bin *.img)", "Logo.bin (*.bin)", "Splash.img (*.img)", "All files (*)"]
        onAccepted: logoFile.loadFile(urlToPath(selectedFile))
    }
    
    FolderDialog {
        id: extractDialog
        title: "Select Export Directory"
        onAccepted: logoFile.extractAll(urlToPath(selectedFolder))
    }
    
    FileDialog {
        id: saveDialog
        title: "Save Logo File"
        nameFilters: ["Logo files (*.bin)", "All files (*)"]
        fileMode: FileDialog.SaveFile
        onAccepted: logoFile.saveFile(urlToPath(selectedFile))
    }
    
    // Project Dialogs (using native system dialogs)
    FolderDialog {
        id: unpackProjectDialog
        title: "Select Project Directory (will be created/must be empty)"
        onAccepted: {
            var projectPath = urlToPath(selectedFolder)
            if (logoFile.filePath !== "") {
                logoFile.unpackToProject(logoFile.filePath, projectPath)
            }
        }
    }
    
    FolderDialog {
        id: openProjectDialog
        title: "Open BootMod Project Folder"
        onAccepted: {
            var projectPath = urlToPath(selectedFolder)
            if (logoFile.isProjectFolder(projectPath)) {
                logoFile.openProject(projectPath)
            } else {
                console.log("Not a valid BootMod project folder")
            }
        }
    }
    
    FileDialog {
        id: exportProjectDialog
        title: "Export Boot Logo File"
        nameFilters: ["Boot Logo files (*.bin *.img)", "Logo.bin (*.bin)", "Splash.img (*.img)", "All files (*)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "bin"
        onAccepted: logoFile.exportProject(urlToPath(selectedFile))
    }
    
    // Main layout
    background: Rectangle {
        color: root.bgColor
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            color: root.surfaceColor
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: root.borderColor
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: root.spacing * 2
                anchors.rightMargin: root.spacing * 2
                anchors.topMargin: root.spacing
                anchors.bottomMargin: root.spacing
                spacing: root.spacing * 1.5
                
                // Logo with SVG
                Image {
                    source: "qrc:/BootMod/res/bootmod_logo.svg"
                    sourceSize.width: 48
                    sourceSize.height: 48
                    smooth: true
                }
                
                // Title section
                ColumnLayout {
                    spacing: 3
                    
                    Text {
                        text: "BootMod"
                        font.pixelSize: 20
                        font.bold: true
                        color: root.textColor
                    }
                    
                    Text {
                        text: logoFile.isLoaded ? logoFile.headerInfo : "Boot Logo Manager · MediaTek & Qualcomm Support"
                        font.pixelSize: 11
                        color: root.textSecondaryColor
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Version badge
                Rectangle {
                    Layout.alignment: Qt.AlignVCenter
                    width: 60
                    height: 24
                    radius: 12
                    color: root.accentColor
                    opacity: 0.9
                    
                    Text {
                        anchors.centerIn: parent
                        text: "v" + appVersion
                        font.pixelSize: 11
                        font.bold: true
                        color: "white"
                    }
                }
            }
        }
        
        // Content
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            // Main content area
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.spacing * 2
                spacing: root.spacing * 2
                
                // File Selection Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    color: root.surfaceColor
                    border.color: root.borderColor
                    border.width: 1
                    radius: root.radius
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: root.spacing * 1.5
                        spacing: root.spacing
                        
                        Text {
                            text: "File Operations"
                            font.pixelSize: 16
                            font.bold: true
                            color: root.textColor
                        }
                        
                        // Open File Row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: root.spacing
                            
                            Text {
                                text: "Logo/Splash File:"
                                font.pixelSize: 13
                                color: root.textColor
                                Layout.preferredWidth: 88
                            }
                            
                            TextField {
                                id: filePathField
                                Layout.fillWidth: true
                                placeholderText: "No file selected"
                                text: logoFile.filePath
                                readOnly: true
                                color: root.textColor
                                
                                background: Rectangle {
                                    color: root.bgColor
                                    border.color: root.borderColor
                                    border.width: 1
                                    radius: root.radius / 2
                                }
                            }
                            
                            Button {
                                text: "Browse..."
                                Layout.preferredWidth: 100
                                Layout.preferredHeight: 32
                                onClicked: logoFile.browseForFile()
                                
                                background: Rectangle {
                                    implicitWidth: 100
                                    implicitHeight: 32
                                    color: parent.hovered ? root.accentHoverColor : root.accentColor
                                    radius: root.radius / 2
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 13
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: root.borderColor
                        }
                        
                        // Action Buttons Row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: root.spacing
                            
                            Button {
                                text: "Unpack to Project"
                                enabled: logoFile.isLoaded
                                Layout.preferredWidth: 140
                                Layout.preferredHeight: 32
                                onClicked: {
                                    var folder = logoFile.browseForFolder()
                                    if (folder !== "" && logoFile.filePath !== "") {
                                        logoFile.unpackToProject(logoFile.filePath, folder)
                                    }
                                }
                                
                                background: Rectangle {
                                    implicitWidth: 140
                                    implicitHeight: 32
                                    color: parent.enabled ? (parent.hovered ? "#2d2d30" : root.surfaceColor) : root.surfaceColor
                                    border.color: parent.enabled ? root.accentColor : root.borderColor
                                    border.width: 1
                                    radius: root.radius / 2
                                    opacity: parent.enabled ? 1.0 : 0.5
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 12
                                    color: parent.enabled ? root.accentColor : root.textSecondaryColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            
                            Button {
                                text: "Open Project"
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 32
                                onClicked: {
                                    var folder = logoFile.browseForFolder()
                                    if (folder !== "") {
                                        if (logoFile.isProjectFolder(folder)) {
                                            logoFile.openProject(folder)
                                        }
                                    }
                                }
                                
                                background: Rectangle {
                                    implicitWidth: 120
                                    implicitHeight: 32
                                    color: parent.hovered ? "#2d2d30" : root.surfaceColor
                                    border.color: root.accentColor
                                    border.width: 1
                                    radius: root.radius / 2
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 12
                                    color: root.accentColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            
                            Button {
                                text: "Export File"
                                enabled: logoFile.isLoaded
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 32
                                onClicked: logoFile.browseForSaveFile()
                                
                                background: Rectangle {
                                    implicitWidth: 120
                                    implicitHeight: 32
                                    color: parent.enabled ? (parent.hovered ? root.accentHoverColor : root.accentColor) : root.surfaceColor
                                    radius: root.radius / 2
                                    opacity: parent.enabled ? 1.0 : 0.5
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: parent.enabled ? "white" : root.textSecondaryColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            
                            Button {
                                text: "Device Preview"
                                enabled: logoFile.isProjectMode && logoFile.logoCount > 0 && logoFile.formatType === "MediaTek"
                                Layout.preferredWidth: 130
                                Layout.preferredHeight: 32
                                onClicked: devicePreviewDialog.startPreview(1)  // Start from frame 1
                                
                                background: Rectangle {
                                    implicitWidth: 130
                                    implicitHeight: 32
                                    color: parent.enabled ? (parent.hovered ? "#3d4f7d" : "#2d3f5f") : root.surfaceColor
                                    border.color: parent.enabled ? "#4a90e2" : root.borderColor
                                    border.width: 1
                                    radius: root.radius / 2
                                    opacity: parent.enabled ? 1.0 : 0.5
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 12
                                    color: parent.enabled ? "#4a90e2" : root.textSecondaryColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            Button {
                                text: "Close Project"
                                enabled: logoFile.isLoaded
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 32
                                onClicked: logoFile.clearFile()
                                
                                background: Rectangle {
                                    implicitWidth: 120
                                    implicitHeight: 32
                                    color: parent.enabled ? (parent.hovered ? "#5a1e1e" : root.surfaceColor) : root.surfaceColor
                                    border.color: parent.enabled ? "#d13438" : root.borderColor
                                    border.width: 1
                                    radius: root.radius / 2
                                    opacity: parent.enabled ? 1.0 : 0.5
                                }
                                
                                contentItem: Text {
                                    text: parent.text
                                    font.pixelSize: 12
                                    color: parent.enabled ? "#d13438" : root.textSecondaryColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }
                
                // Info Panel (Stats)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    visible: logoFile.isLoaded
                    color: root.surfaceColor
                    border.color: root.borderColor
                    border.width: 1
                    radius: root.radius
                    
                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: root.spacing * 2
                        anchors.rightMargin: root.spacing * 2
                        spacing: root.spacing * 3
                        height: parent.height
                        
                        // Logo Count
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            
                            Rectangle {
                                width: 32
                                height: 32
                                radius: 16
                                color: root.accentColor
                                
                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/BootMod/res/icons/chart.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                }
                            }
                            
                            ColumnLayout {
                                spacing: 2
                                Layout.alignment: Qt.AlignVCenter
                                
                                Text {
                                    text: logoFile.logoCount
                                    font.pixelSize: 18
                                    font.bold: true
                                    color: root.textColor
                                }
                                
                                Text {
                                    text: "Logos"
                                    font.pixelSize: 10
                                    color: root.textSecondaryColor
                                }
                            }
                        }
                        
                        Rectangle {
                            width: 1
                            height: 40
                            Layout.alignment: Qt.AlignVCenter
                            color: root.borderColor
                        }
                        
                        // Project Mode Status
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            
                            Rectangle {
                                width: 32
                                height: 32
                                radius: 16
                                color: logoFile.isProjectMode ? "#4ec9b0" : root.borderColor
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: logoFile.isProjectMode ? "✓" : "○"
                                    font.pixelSize: 16
                                    font.bold: true
                                    color: "white"
                                }
                            }
                            
                            ColumnLayout {
                                spacing: 2
                                Layout.alignment: Qt.AlignVCenter
                                
                                Text {
                                    text: logoFile.isProjectMode ? "Project Mode" : "File Mode"
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: root.textColor
                                }
                                
                                Text {
                                    text: logoFile.isProjectMode ? "Can replace logos" : "Read-only"
                                    font.pixelSize: 10
                                    color: root.textSecondaryColor
                                }
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        // File Info
                        ColumnLayout {
                            spacing: 4
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            
                            Text {
                                text: logoFile.filePath ? logoFile.filePath.split('/').pop() : ""
                                font.pixelSize: 12
                                font.bold: true
                                color: root.textColor
                                Layout.alignment: Qt.AlignRight
                                elide: Text.ElideMiddle
                                Layout.maximumWidth: 250
                            }
                            
                            RowLayout {
                                spacing: 5
                                Layout.alignment: Qt.AlignRight
                                
                                Image {
                                    source: logoFile.isProjectMode ? "qrc:/BootMod/res/icons/folder.svg" : "qrc:/BootMod/res/icons/file.svg"
                                    width: 10
                                    height: 10
                                    sourceSize: Qt.size(10, 10)
                                    Layout.alignment: Qt.AlignVCenter
                                }
                                
                                Text {
                                    text: logoFile.isProjectMode ? "Project Folder" : "Binary File"
                                    font.pixelSize: 10
                                    color: root.textSecondaryColor
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                        }
                    }
                }
                
                // Logo Grid
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: logoFile.isLoaded
                    clip: true
                    
                    contentWidth: availableWidth
                    
                    GridView {
                        id: gridView
                        cellWidth: 200
                        cellHeight: 240
                        model: logoModel
                        
                        delegate: Rectangle {
                            id: logoCard
                            width: gridView.cellWidth - root.spacing
                            height: gridView.cellHeight - root.spacing
                            color: mouseArea.containsMouse ? "#2d2d30" : root.surfaceColor
                            border.color: root.borderColor
                            border.width: 1
                            radius: root.radius
                            
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: root.spacing
                                spacing: root.spacing / 2
                                
                                // Thumbnail
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.preferredHeight: 140
                                    color: "#1a1a1a"
                                    radius: root.radius / 2
                                    
                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        source: "image://thumbnail/" + logoIndex
                                        fillMode: Image.PreserveAspectFit
                                        smooth: true
                                        cache: false
                                    }
                                }
                                
                                // Info
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    
                                    Text {
                                        text: "Logo #" + logoIndex
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: root.textColor
                                    }
                                    
                                    Text {
                                        text: model.width + "×" + model.height + " · " + format
                                        font.pixelSize: 11
                                        color: root.textSecondaryColor
                                    }
                                    
                                    Text {
                                        text: (size / 1024).toFixed(1) + " KB"
                                        font.pixelSize: 11
                                        color: root.textSecondaryColor
                                    }
                                }
                                
                                // Actions
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    
                                    Button {
                                        Layout.fillWidth: true
                                        text: "Export"
                                        font.pixelSize: 11
                                        
                                        onClicked: logoFile.browseAndExtractLogo(logoIndex)
                                        
                                        background: Rectangle {
                                            implicitHeight: 28
                                            color: parent.hovered ? root.borderColor : "transparent"
                                            border.color: root.borderColor
                                            border.width: 1
                                            radius: root.radius / 2
                                        }
                                        
                                        contentItem: Text {
                                            text: parent.text
                                            font.pixelSize: 11
                                            color: root.textColor
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                    
                                    Button {
                                        Layout.fillWidth: true
                                        text: "Replace"
                                        font.pixelSize: 11
                                        enabled: logoFile.isProjectMode
                                        
                                        onClicked: logoFile.browseAndReplaceLogo(logoIndex)
                                        
                                        background: Rectangle {
                                            implicitHeight: 28
                                            color: parent.enabled ? (parent.hovered ? root.borderColor : "transparent") : "transparent"
                                            border.color: parent.enabled ? root.borderColor : "#404040"
                                            border.width: 1
                                            radius: root.radius / 2
                                            opacity: parent.enabled ? 1.0 : 0.3
                                        }
                                        
                                        contentItem: Text {
                                            text: parent.text
                                            font.pixelSize: 11
                                            color: parent.enabled ? root.textColor : root.textSecondaryColor
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        
                                        ToolTip.visible: !logoFile.isProjectMode && hovered
                                        ToolTip.text: "Replace only works in project mode. Use 'Unpack to Project' first."
                                        ToolTip.delay: 500
                                    }
                                }
                            }
                            
                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.NoButton  // Don't capture any mouse clicks
                            }
                        }
                    }
                }
                
                // Empty state
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !logoFile.isLoaded
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        width: 400
                        spacing: root.spacing * 2
                        
                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            width: 120
                            height: 120
                            radius: 60
                            color: root.surfaceColor
                            border.color: root.borderColor
                            border.width: 2
                            
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/BootMod/res/icons/package.svg"
                                width: 64
                                height: 64
                                sourceSize: Qt.size(64, 64)
                            }
                        }
                        
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "No file loaded"
                            font.pixelSize: 24
                            font.bold: true
                            color: root.textColor
                        }
                        
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Click 'Browse...' above to open a logo.bin or splash.img file"
                            font.pixelSize: 14
                            color: root.textSecondaryColor
                        }
                    }
                }
            }
        }
        
        // Status bar
        Rectangle {
            id: statusBar
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: root.surfaceColor
            
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: root.borderColor
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: root.spacing
                anchors.rightMargin: root.spacing
                spacing: root.spacing
                
                Text {
                    id: statusText
                    Layout.fillWidth: true
                    font.pixelSize: 12
                    color: root.textSecondaryColor
                    text: logoFile.isLoaded ? logoFile.logoCount + " logos loaded" : "Ready"
                    
                    property color normalColor: root.textSecondaryColor
                    property color successColor: root.successColor
                    property color errorColor: root.errorColor
                    
                    function showSuccess(message) {
                        text = message
                        color = successColor
                        resetTimer.restart()
                    }
                    
                    function showError(message) {
                        text = message
                        color = errorColor
                        resetTimer.restart()
                    }
                    
                    Timer {
                        id: resetTimer
                        interval: 5000
                        onTriggered: {
                            statusText.color = statusText.normalColor
                            statusText.text = logoFile.isLoaded ? logoFile.logoCount + " logos loaded" : "Ready"
                        }
                    }
                }
                
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    Layout.topMargin: 6
                    Layout.bottomMargin: 6
                    color: root.borderColor
                }
                
                Button {
                    text: "About"
                    flat: true
                    Layout.preferredHeight: 24
                    onClicked: aboutDialog.open()
                    
                    background: Rectangle {
                        color: parent.hovered ? "#2d2d30" : "transparent"
                        radius: 4
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 11
                        color: root.accentColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Text {
                    text: "v" + appVersion
                    font.pixelSize: 11
                    color: root.textSecondaryColor
                }
            }
        }
    }
    
    // About Dialog
    AboutDialog {
        id: aboutDialog
    }
    
    // Device Preview Dialog
    DevicePreviewDialog {
        id: devicePreviewDialog
        
        // Connect to bitmap editor signal to refresh thumbnails
        onImageEdited: function(path) {
            console.log("Main window: Image edited, refreshing thumbnails for:", path)
            refreshThumbnails()
        }
    }
    
    // Function to refresh thumbnail cache
    function refreshThumbnails() {
        // Force the thumbnail provider to reload by rescanning project images
        if (logoFile.isLoaded && logoFile.isProjectMode) {
            console.log("Rescanning project images to refresh thumbnails")
            logoFile.rescanProjectImages()
        }
    }
    
    // Drag & Drop
    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0) {
                var path = drop.urls[0].toString()
                if (path.startsWith("file://")) {
                    path = path.substring(7)
                }
                logoFile.loadFile(path)
            }
        }
    }
}
