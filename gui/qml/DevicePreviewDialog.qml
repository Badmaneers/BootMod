import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.folderlistmodel
import BootMod

ApplicationWindow {
    id: devicePreviewDialog
    title: "Device Preview - Multi-Layer Boot Animation"
    width: 1400
    height: 900
    visible: false
    
    // Enable drag and drop from other windows
    flags: Qt.Window
    
    // Add property to track if window is open for drag and drop
    property bool opened: visible
    
    // Signals to notify when images are edited
    signal imageEdited(string path)
    
    property int currentFrame: 1  // Start from frame 1 (logo_1)
    property int totalFrames: 0
    property bool isPlaying: false
    property int playbackSpeed: 100
    property int canvasWidth: 720
    property int canvasHeight: 1280
    property real canvasScale: 1.0
    property var imageDimensions: ({}) // Map of frame number -> {width, height}
    property int userLayerCount: 0 // Count of user layers (excludes auto-animation)
    property bool dimensionsReady: false  // Flag to prevent race condition
    
    // Use LayerManager's selectedLayerIndex instead of local property
    property alias selectedLayerIndex: layerManager.selectedLayerIndex
    
    // Layer manager instance
    LayerManager {
        id: layerManager
        
        onLayersChanged: {
            // Update user layer count (exclude auto-animation)
            var count = 0
            for (var i = 0; i < layerManager.layerCount; i++) {
                var layer = layerManager.getLayer(i)
                if (layer && layer.endFrame !== -1) {
                    count++
                }
            }
            userLayerCount = count
        }
    }
    
    // Command manager for undo/redo
    CommandManager {
        id: commandManager
    }
    
    // Bitmap editor wrapper
    BitmapEditorWrapper {
        id: bitmapEditor
        
        onImageSaved: function(path) {
            console.log("Bitmap editor saved image:", path)
            // Force refresh of all layer images by triggering updateImage()
            refreshAllLayers()
            // Emit signal so Main.qml can also refresh
            devicePreviewDialog.imageEdited(path)
        }
        
        onEditorClosed: function() {
            console.log("Bitmap editor closed")
            // Also refresh when closed in case the save happened earlier
            refreshAllLayers()
        }
    }
    
    // Function to refresh all layer images
    function refreshAllLayers() {
        console.log("Refreshing all layers...")
        // Trigger updateImage for all layers
        for (var i = 0; i < layersRepeater.count; i++) {
            var layerItem = layersRepeater.itemAt(i)
            if (layerItem && layerItem.layerData) {
                var image = layerItem.children[1] // The Image is the second child (after Rectangle selection border)
                if (image && image.updateImage) {
                    // Force cache clear by appending timestamp
                    var oldSource = image.source.toString()
                    if (oldSource.indexOf("?") > -1) {
                        oldSource = oldSource.substring(0, oldSource.indexOf("?"))
                    }
                    image.source = oldSource + "?" + Date.now()
                    console.log("  Refreshed layer", i, "source:", image.source)
                }
            }
        }
    }
    
    // Folder model to scan image files
    FolderListModel {
        id: imagesFolderModel
        nameFilters: ["logo_*.png"]
        showDirs: false
        
        onStatusChanged: {
            if (status === FolderListModel.Ready) {
                console.log("FolderListModel ready, count:", count)
                // Trigger dimension scan when ready
                if (devicePreviewDialog.visible) {
                    scanImageDimensions()
                    dimensionsReady = true  // Mark as ready
                    console.log("Image dimensions ready, count:", Object.keys(imageDimensions).length)
                }
            }
        }
    }
    
    Component.onCompleted: {
        currentFrame = 1  // Ensure it starts at 1
    }
    
    function scanImageDimensions() {
        // Build a map of frame number -> {width, height} by scanning image filenames
        imageDimensions = {}
        
        if (!logoFile.isProjectMode || !logoFile.projectPath) {
            return
        }
        
        console.log("Scanning", imagesFolderModel.count, "image files...")
        
        // Parse filenames to extract dimensions
        // Format: logo_{number}_{width}x{height}.png
        var regex = /logo_(\d+)_(\d+)x(\d+)\.png/
        
        for (var i = 0; i < imagesFolderModel.count; i++) {
            var filename = imagesFolderModel.get(i, "fileName")
            var match = regex.exec(filename)
            
            if (match && match.length >= 4) {
                var logoIndex = parseInt(match[1])
                var width = parseInt(match[2])
                var height = parseInt(match[3])
                
                imageDimensions[logoIndex] = {width: width, height: height}
                console.log("Scanned:", filename, "->", logoIndex, width + "x" + height)
            }
        }
        
        console.log("Total images scanned:", Object.keys(imageDimensions).length)
    }
    
    function startPreview(startFrame) {
        currentFrame = startFrame >= 1 ? startFrame : 1
        totalFrames = logoFile.logoCount
        
        // Reset dimensions ready flag
        dimensionsReady = false
        
        // Set up folder model to scan images
        if (logoFile.isProjectMode && logoFile.projectPath) {
            var imagesPath = logoFile.projectPath + "/images"
            imagesFolderModel.folder = "file://" + imagesPath
            console.log("Setting folder model to:", imagesPath)
        }
        
        // Initial scan attempt (will be called again when folder model is ready)
        if (imagesFolderModel.status === FolderListModel.Ready) {
            scanImageDimensions()
            dimensionsReady = true
            console.log("Initial scan complete:", Object.keys(imageDimensions).length, "images")
        } else {
            console.log("Folder model not ready yet, will scan when ready")
        }
        
        // Get the dimensions of the first logo for canvas sizing
        if (totalFrames > 0) {
            var firstWidth = logoModel.data(logoModel.index(0, 0), 258) // WidthRole
            var firstHeight = logoModel.data(logoModel.index(0, 0), 259) // HeightRole
            
            if (firstWidth > 0 && firstHeight > 0) {
                canvasWidth = firstWidth
                canvasHeight = firstHeight
                
                // Auto-scale to fit in preview area (max 600px height)
                var maxHeight = 600
                if (canvasHeight > maxHeight) {
                    canvasScale = maxHeight / canvasHeight
                } else {
                    canvasScale = 1.0
                }
            }
        }
        
        // Load saved layers or create default
        var layersFile = logoFile.projectPath + "/.layers.json"
        if (!layerManager.loadFromFile(layersFile)) {
            // Create default auto-animating layer
            var defaultLayer = layerManager.createLayer()
            defaultLayer.logoIndex = 1
            defaultLayer.name = "Auto Animation"
            defaultLayer.xPosition = 0
            defaultLayer.yPosition = 0
            defaultLayer.visible = true
            defaultLayer.startFrame = 1
            defaultLayer.endFrame = -1  // All frames = auto-animate
        }
        
        devicePreviewDialog.show()
        devicePreviewDialog.raise()
        devicePreviewDialog.requestActivate()
    }
    
    function addLogoLayer(logoIndex) {
        console.log("addLogoLayer called with logoIndex:", logoIndex)
        
        if (!logoFile.isProjectMode) {
            console.log("Not in project mode, cannot add layer")
            return
        }
        
        if (logoIndex < 1 || logoIndex > totalFrames) {
            console.log("Invalid logo index:", logoIndex, "totalFrames:", totalFrames)
            return
        }
        
        var layer = layerManager.createLayer()
        layer.logoIndex = logoIndex
        layer.name = "Logo " + logoIndex
        layer.xPosition = 50
        layer.yPosition = 50
        layer.startFrame = 1
        layer.endFrame = totalFrames
        layer.visible = true
        layer.opacity = 1.0
        
        console.log("Created new layer:", layer.name, "at index:", layerManager.layerCount - 1)
        
        // Select the newly created layer
        layerManager.selectedLayerIndex = layerManager.layerCount - 1
        layersList.currentIndex = layerManager.layerCount - 1
        
        // Save layers
        saveLayers()
    }
    
    function play() {
        isPlaying = true
        animationTimer.start()
    }
    
    function pause() {
        isPlaying = false
        animationTimer.stop()
    }
    
    function stop() {
        isPlaying = false
        animationTimer.stop()
        currentFrame = 1  // Reset to frame 1 (logo_1)
    }
    
    function saveLayers() {
        var layersFile = logoFile.projectPath + "/.layers.json"
        if (layerManager.saveToFile(layersFile)) {
            console.log("Layers saved")
        }
    }
    
    function mergeLayers(upperIndex, lowerIndex) {
        // Use C++ backend for true Photoshop-style image compositing
        var upperLayer = layerManager.getLayer(upperIndex)
        var lowerLayer = layerManager.getLayer(lowerIndex)
        
        if (!upperLayer || !lowerLayer) {
            console.log("Cannot merge: invalid layer indices")
            return false
        }
        
        // Can't merge auto-animation layer
        if (upperLayer.endFrame === -1 || lowerLayer.endFrame === -1) {
            console.log("Cannot merge auto-animation layer")
            return false
        }
        
        console.log("==== MERGE STARTING ====")
        console.log("Upper layer:", upperLayer.name, "logoIndex:", upperLayer.logoIndex)
        console.log("Lower layer:", lowerLayer.name, "logoIndex:", lowerLayer.logoIndex)
        console.log("Canvas size:", canvasWidth, "x", canvasHeight)
        console.log("Project path:", logoFile.projectPath)
        
        // Call C++ backend to composite images
        var success = layerManager.mergeLayers(upperIndex, lowerIndex, logoFile.projectPath, canvasWidth, canvasHeight)
        
        if (success) {
            console.log("✓ C++ merge completed successfully!")
            
            // Get the merged layer to see its new properties
            var mergedLayer = layerManager.getLayer(lowerIndex > upperIndex ? lowerIndex - 1 : lowerIndex)
            if (mergedLayer) {
                console.log("  Merged layer:")
                console.log("    Name:", mergedLayer.name)
                console.log("    Position:", mergedLayer.xPosition, ",", mergedLayer.yPosition)
                console.log("    Custom image path:", mergedLayer.customImagePath)
            }
            
            // Force UI update
            layerManager.layersChanged()
            
            // Select the merged layer (adjusted index after removal)
            layersList.currentIndex = lowerIndex > upperIndex ? lowerIndex - 1 : lowerIndex
            selectedLayerIndex = layersList.currentIndex
            
            console.log("==== MERGE COMPLETE ====")
        } else {
            console.log("✗ Merge failed - check C++ console output")
        }
        
        return success
    }
    
    // Timer to refresh image dimensions after merge
    Timer {
        id: refreshTimer
        interval: 500  // Increased delay to ensure file system sync
        repeat: false
        onTriggered: {
            console.log("==== REFRESH TIMER TRIGGERED ====")
            
            // First, rescan the project to update logo count
            console.log("Step 1: Rescanning project to update logo count...")
            var oldCount = totalFrames
            logoFile.rescanProjectImages()
            totalFrames = logoFile.logoCount
            console.log("  Total frames updated:", oldCount, "->", totalFrames)
            
            // Then, scan image dimensions
            console.log("Step 2: Rescanning image dimensions...")
            console.log("  Folder model count:", imagesFolderModel.count)
            scanImageDimensions()
            console.log("  After scan, imageDimensions has", Object.keys(imageDimensions).length, "entries")
            
            // Force update of all layer images
            console.log("Step 3: Forcing layer refresh...")
            layerManager.layersChanged()
            
            // Also manually trigger update on visible layers
            for (var i = 0; i < layerManager.layerCount; i++) {
                var layer = layerManager.getLayer(i)
                if (layer) {
                    console.log("  Layer", i, "logoIndex:", layer.logoIndex, "name:", layer.name)
                    // Trigger change signal
                    layer.logoIndexChanged()
                }
            }
            console.log("==== REFRESH COMPLETE ====")
        }
    }
    
    Timer {
        id: animationTimer
        interval: devicePreviewDialog.playbackSpeed
        repeat: true
        running: false
        onTriggered: {
            currentFrame++
            if (currentFrame > totalFrames) {
                currentFrame = 1
            }
        }
    }
    
    // Main content
    Item {
        anchors.fill: parent
        anchors.bottomMargin: windowToolbar.height
    
        // Main content area
        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 15
            
            // Left side - Canvas preview
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.55
                spacing: 10
            
            // Canvas area
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 400
                
                // Canvas container
                Rectangle {
                    id: canvasContainer
                    anchors.centerIn: parent
                    width: canvasWidth * canvasScale
                    height: canvasHeight * canvasScale
                    color: "#000000"
                    border.color: "#555555"
                    border.width: 2
                    clip: true  // Clip layers to canvas bounds
                    
                    // Canvas info overlay
                    Rectangle {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 5
                        width: canvasInfo.width + 10
                        height: canvasInfo.height + 6
                        color: "#000000"
                        opacity: 0.7
                        z: 1000
                        
                        Text {
                            id: canvasInfo
                            anchors.centerIn: parent
                            text: canvasWidth + "×" + canvasHeight + " @ " + (canvasScale * 100).toFixed(0) + "%"
                            color: "#00ff00"
                            font.pixelSize: 10
                            font.family: "monospace"
                        }
                    }
                    
                    // Layer rendering
                    Repeater {
                        model: layerManager.layerCount
                        
                        delegate: Item {
                            id: layerItem
                            property var layerData: layerManager.getLayer(index)
                            property bool isSelected: selectedLayerIndex === index
                            property bool isDragging: false
                            property real storedX: 0
                            property real storedY: 0
                            
                            visible: layerData && layerData.visible && layerData.isVisibleAtFrame(currentFrame)
                            z: layerData ? layerData.zIndex : 0
                            width: layerImage.width
                            height: layerImage.height
                            
                            // Position controlled by storedX/Y
                            x: storedX
                            y: storedY
                            
                            // Update stored position when x/y change (from drag or other sources)
                            onXChanged: {
                                if (isDragging) {
                                    storedX = x
                                }
                            }
                            onYChanged: {
                                if (isDragging) {
                                    storedY = y
                                }
                            }
                            
                            Component.onCompleted: {
                                if (layerData) {
                                    storedX = layerData.xPosition * canvasScale
                                    storedY = layerData.yPosition * canvasScale
                                }
                            }
                            
                            // Listen for position changes from the layer object itself
                            Connections {
                                target: layerData
                                function onXPositionChanged() {
                                    if (!layerItem.isDragging) {
                                        layerItem.storedX = layerData.xPosition * canvasScale
                                    }
                                }
                                function onYPositionChanged() {
                                    if (!layerItem.isDragging) {
                                        layerItem.storedY = layerData.yPosition * canvasScale
                                    }
                                }
                            }
                            
                            // Selection border
                            Rectangle {
                                anchors.fill: parent
                                color: "transparent"
                                border.color: "#00ffff"
                                border.width: isSelected ? 2 : 0
                                z: 1000
                            }
                            
                            Image {
                                id: layerImage
                                
                                function updateImage() {
                                    if (!layerItem.layerData) {
                                        console.log("Layer", index, "- No layerData")
                                        source = ""
                                        return
                                    }
                                    
                                    // Wait for dimensions to be ready to avoid race condition
                                    if (!devicePreviewDialog.dimensionsReady && !layerItem.layerData.customImagePath) {
                                        console.log("Layer", index, "- Dimensions not ready yet, will retry...")
                                        Qt.callLater(updateImage)  // Retry on next event loop
                                        return
                                    }
                                    
                                    // Check if this is a merged layer with custom image path
                                    if (layerItem.layerData.customImagePath && layerItem.layerData.customImagePath !== "") {
                                        console.log("Layer", index, "- Loading merged image:", layerItem.layerData.customImagePath)
                                        source = "file://" + layerItem.layerData.customImagePath
                                        return
                                    }
                                    
                                    // Determine which logo to display
                                    var displayIndex = layerItem.layerData.logoIndex
                                    console.log("Layer", index, "- logoIndex:", displayIndex, "name:", layerItem.layerData.name)
                                    
                                    // Auto-animation: if layer covers all frames (startFrame=1, endFrame=-1), use currentFrame
                                    if (layerItem.layerData.startFrame === 1 && layerItem.layerData.endFrame === -1) {
                                        displayIndex = devicePreviewDialog.currentFrame
                                        console.log("  Auto-animation layer, using currentFrame:", displayIndex)
                                    }
                                    
                                    // Don't load if displayIndex is invalid (still initializing)
                                    if (displayIndex < 1 || displayIndex > totalFrames) {
                                        console.log("  Invalid displayIndex:", displayIndex, "(totalFrames:", totalFrames + ")")
                                        return
                                    }
                                    
                                    // Load the image with its actual dimensions
                                    if (logoFile.isProjectMode && logoFile.projectPath) {
                                        var dims = imageDimensions[displayIndex]
                                        console.log("  Looking for dimensions for logo", displayIndex)
                                        console.log("  imageDimensions available:", Object.keys(imageDimensions).length, "entries")
                                        
                                        if (dims) {
                                            var imagePath = "file://" + logoFile.projectPath + "/images/logo_" + displayIndex + "_" + dims.width + "x" + dims.height + ".png"
                                            console.log("  Loading with dims:", imagePath)
                                            source = imagePath
                                        } else {
                                            // Fallback to canvas dimensions
                                            var fallbackPath = "file://" + logoFile.projectPath + "/images/logo_" + displayIndex + "_" + canvasWidth + "x" + canvasHeight + ".png"
                                            console.log("  No dims found, fallback:", fallbackPath)
                                            source = fallbackPath
                                        }
                                    } else {
                                        source = "image://thumbnail/" + displayIndex
                                    }
                                    console.log("  Final source:", source)
                                }
                                
                                // Update when currentFrame changes
                                Connections {
                                    target: devicePreviewDialog
                                    function onCurrentFrameChanged() {
                                        layerImage.updateImage()
                                    }
                                }
                                
                                // Update when layer logoIndex changes (important for merged layers)
                                Connections {
                                    target: layerItem.layerData
                                    function onLogoIndexChanged() {
                                        console.log("★ Logo index changed for layer", index, "to", layerItem.layerData.logoIndex, "- updating image")
                                        layerImage.updateImage()
                                    }
                                    function onCustomImagePathChanged() {
                                        console.log("★ Custom image path changed for layer", index, "- updating image")
                                        layerImage.updateImage()
                                    }
                                }
                                
                                // Initial load
                                Component.onCompleted: updateImage()
                                fillMode: Image.PreserveAspectFit
                                opacity: layerItem.layerData ? layerItem.layerData.opacity : 1.0
                                smooth: true
                                cache: false
                                asynchronous: true
                                
                                // Smart sizing logic - using properties not bindings so context menu can override
                                property bool manuallyResized: false
                                
                                width: {
                                    if (manuallyResized) return width // Keep manual size
                                    if (implicitWidth <= 0 || implicitHeight <= 0) return 0
                                    
                                    if (implicitWidth >= canvasWidth || implicitHeight >= canvasHeight) {
                                        return canvasWidth * canvasScale
                                    }
                                    return implicitWidth
                                }
                                height: {
                                    if (manuallyResized) return height // Keep manual size
                                    if (implicitWidth <= 0 || implicitHeight <= 0) return 0
                                    
                                    if (implicitWidth >= canvasWidth || implicitHeight >= canvasHeight) {
                                        return canvasHeight * canvasScale
                                    }
                                    return implicitHeight
                                }
                                
                                // Center small images
                                anchors.centerIn: (implicitWidth < canvasWidth && implicitHeight < canvasHeight) ? parent : undefined
                                
                                onStatusChanged: {
                                    if (status === Image.Error) {
                                        console.log("Image load error for logo index:", layerData ? layerData.logoIndex : "undefined", "source:", source)
                                    } else if (status === Image.Ready) {
                                        console.log("Image loaded successfully for logo index:", layerData ? layerData.logoIndex : "undefined", "size:", implicitWidth + "x" + implicitHeight)
                                    }
                                }
                            }
                            
                            // Render linked layers visually composited
                            Repeater {
                                model: layerData && layerData.linkedLayers ? layerData.linkedLayers.length : 0
                                delegate: Image {
                                    property var linkedLayer: layerData && layerData.linkedLayers ? layerData.linkedLayers[index] : null
                                    visible: linkedLayer !== null
                                    x: linkedLayer ? (linkedLayer.xPosition * canvasScale) : 0
                                    y: linkedLayer ? (linkedLayer.yPosition * canvasScale) : 0
                                    opacity: linkedLayer ? linkedLayer.opacity : 1.0
                                    z: linkedLayer ? linkedLayer.zIndex : 0
                                    source: {
                                        if (!linkedLayer) return ""
                                        var displayIndex = linkedLayer.logoIndex
                                        if (displayIndex < 1 || displayIndex > totalFrames) return ""
                                        if (!logoFile.isProjectMode || !logoFile.projectPath) return ""
                                        var dims = imageDimensions[displayIndex]
                                        if (dims) {
                                            return "file://" + logoFile.projectPath + "/images/logo_" + displayIndex + "_" + dims.width + "x" + dims.height + ".png"
                                        } else {
                                            return "file://" + logoFile.projectPath + "/images/logo_" + displayIndex + "_" + canvasWidth + "x" + canvasHeight + ".png"
                                        }
                                    }
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                    cache: false
                                    asynchronous: true
                                }
                            }
                            
                            // Drag handler and context menu
                            MouseArea {
                                anchors.fill: parent
                                drag.target: layerItem
                                drag.minimumX: 0
                                drag.maximumX: (canvasWidth * canvasScale) - layerItem.width
                                drag.minimumY: 0
                                drag.maximumY: (canvasHeight * canvasScale) - layerItem.height
                                cursorShape: Qt.OpenHandCursor
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                
                                property point dragStartPos: Qt.point(0, 0)
                                
                                onPressed: (mouse) => {
                                    if (mouse.button === Qt.LeftButton) {
                                        cursorShape = Qt.ClosedHandCursor
                                        layerItem.isDragging = true
                                        selectedLayerIndex = index
                                        layersList.currentIndex = index
                                        
                                        // Store starting position for undo
                                        if (layerData) {
                                            dragStartPos = Qt.point(layerData.xPosition, layerData.yPosition)
                                        }
                                    }
                                }
                                
                                onPositionChanged: (mouse) => {
                                    if (mouse.buttons & Qt.LeftButton) {
                                        if (layerData) {
                                            // Update position live during drag
                                            layerData.xPosition = Math.round(layerItem.storedX / canvasScale)
                                            layerData.yPosition = Math.round(layerItem.storedY / canvasScale)
                                        }
                                    }
                                }
                                
                                onReleased: (mouse) => {
                                    if (mouse.button === Qt.LeftButton) {
                                        cursorShape = Qt.OpenHandCursor
                                        if (layerData) {
                                            // Constrain within bounds
                                            layerItem.storedX = Math.max(0, Math.min(layerItem.storedX, (canvasWidth * canvasScale) - layerItem.width))
                                            layerItem.storedY = Math.max(0, Math.min(layerItem.storedY, (canvasHeight * canvasScale) - layerItem.height))
                                            
                                            // Save final unscaled position
                                            var newX = Math.round(layerItem.storedX / canvasScale)
                                            var newY = Math.round(layerItem.storedY / canvasScale)
                                            layerData.xPosition = newX
                                            layerData.yPosition = newY
                                            
                                            // Create undo command if position changed
                                            var newPos = Qt.point(newX, newY)
                                            if (dragStartPos.x !== newPos.x || dragStartPos.y !== newPos.y) {
                                                commandManager.executeMoveCommand(layerManager, index, dragStartPos, newPos)
                                            }
                                        }
                                        layerItem.isDragging = false
                                    }
                                }
                                
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        selectedLayerIndex = index
                                        layersList.currentIndex = index
                                        console.log("Right-click on layer", index, "layerData:", layerItem.layerData)
                                        if (layerItem.layerData) {
                                            console.log("  endFrame:", layerItem.layerData.endFrame)
                                            console.log("  isAutoAnimation:", layerItem.layerData.endFrame === -1)
                                        }
                                        layerContextMenu.popup()
                                    }
                                }
                            }
                            
                            // Context menu for layer operations
                            Menu {
                                id: layerContextMenu
                                
                                // Disable for auto-animation layer (first layer with endFrame == -1)
                                property bool isAutoAnimation: layerItem.layerData && layerItem.layerData.endFrame === -1
                                
                                onAboutToShow: {
                                    console.log("Menu opening, isAutoAnimation:", isAutoAnimation)
                                    console.log("  layerData exists:", !!layerItem.layerData)
                                    if (layerItem.layerData) {
                                        console.log("  endFrame value:", layerItem.layerData.endFrame)
                                    }
                                }
                                
                                MenuItem {
                                    text: "Scale to Fit Canvas"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        layerImage.manuallyResized = true
                                        layerImage.width = canvasWidth * canvasScale
                                        layerImage.height = canvasHeight * canvasScale
                                        console.log("Scaled to fit canvas:", layerImage.width, "x", layerImage.height)
                                    }
                                }
                                
                                MenuItem {
                                    text: "Reset to Original Size"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        layerImage.manuallyResized = false
                                        console.log("Reset to original:", layerImage.width, "x", layerImage.height)
                                    }
                                }
                                
                                MenuItem {
                                    text: "Scale 50%"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        layerImage.manuallyResized = true
                                        layerImage.width = layerImage.width * 0.5
                                        layerImage.height = layerImage.height * 0.5
                                        console.log("Scaled 50%:", layerImage.width, "x", layerImage.height)
                                    }
                                }
                                
                                MenuItem {
                                    text: "Scale 200%"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        layerImage.manuallyResized = true
                                        var newWidth = layerImage.width * 2
                                        var newHeight = layerImage.height * 2
                                        
                                        // Don't exceed canvas bounds
                                        var maxWidth = (canvasWidth * canvasScale) - layerItem.x
                                        var maxHeight = (canvasHeight * canvasScale) - layerItem.y
                                        
                                        layerImage.width = Math.min(newWidth, maxWidth)
                                        layerImage.height = Math.min(newHeight, maxHeight)
                                        console.log("Scaled 200%:", layerImage.width, "x", layerImage.height)
                                    }
                                }
                                
                                MenuItem {
                                    text: "Custom Scale..."
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        scaleDialog.open()
                                    }
                                }
                                
                                MenuSeparator {}
                                
                                MenuItem {
                                    text: "Center on Canvas"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        layerItem.x = ((canvasWidth * canvasScale) - layerItem.width) / 2
                                        layerItem.y = ((canvasHeight * canvasScale) - layerItem.height) / 2
                                        layerItem.layerData.xPosition = Math.round(layerItem.x / canvasScale)
                                        layerItem.layerData.yPosition = Math.round(layerItem.y / canvasScale)
                                        console.log("Centered at:", layerItem.x, layerItem.y)
                                    }
                                }
                                
                                MenuSeparator {}
                                
                                MenuItem {
                                    text: layerItem.layerData && layerItem.layerData.visible ? "Hide Layer" : "Show Layer"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        if (layerItem.layerData) {
                                            layerItem.layerData.visible = !layerItem.layerData.visible
                                            console.log("Layer visibility:", layerItem.layerData.visible)
                                        }
                                    }
                                }
                                
                                MenuSeparator {}
                                
                                MenuItem {
                                    text: "Merge Down"
                                    enabled: {
                                        if (layerContextMenu.isAutoAnimation) return false
                                        
                                        // Check if there's a layer below
                                        var nextVisibleIndex = -1
                                        for (var i = index + 1; i < layerManager.layerCount; i++) {
                                            var layer = layerManager.getLayer(i)
                                            if (layer && layer.endFrame !== -1) {
                                                nextVisibleIndex = i
                                                break
                                            }
                                        }
                                        return nextVisibleIndex !== -1
                                    }
                                    onTriggered: {
                                        // Find next visible layer
                                        var nextVisibleIndex = -1
                                        for (var i = index + 1; i < layerManager.layerCount; i++) {
                                            var layer = layerManager.getLayer(i)
                                            if (layer && layer.endFrame !== -1) {
                                                nextVisibleIndex = i
                                                break
                                            }
                                        }
                                        
                                        if (nextVisibleIndex !== -1) {
                                            mergeLayers(index, nextVisibleIndex)
                                        }
                                    }
                                }
                                
                                MenuItem {
                                    text: "Delete Layer"
                                    enabled: !layerContextMenu.isAutoAnimation
                                    onTriggered: {
                                        console.log("Deleting layer at index:", index)
                                        var layer = layerManager.getLayer(index)
                                        if (layer) {
                                            commandManager.executeRemoveCommand(layerManager, index, layer)
                                        }
                                    }
                                }
                            }
                            
                            // Custom scale dialog
                            Dialog {
                                id: scaleDialog
                                title: "Custom Scale"
                                modal: true
                                anchors.centerIn: parent
                                
                                ColumnLayout {
                                    spacing: 10
                                    
                                    Label {
                                        text: "Enter scale percentage (10-500%):"
                                    }
                                    
                                    RowLayout {
                                        TextField {
                                            id: scaleTextField
                                            Layout.preferredWidth: 100
                                            text: "100"
                                            placeholderText: "100"
                                            selectByMouse: true
                                            
                                            validator: IntValidator {
                                                bottom: 10
                                                top: 500
                                            }
                                            
                                            onAccepted: {
                                                applyScaleButton.clicked()
                                            }
                                            
                                            Component.onCompleted: {
                                                selectAll()
                                                forceActiveFocus()
                                            }
                                        }
                                        
                                        Label {
                                            text: "%"
                                        }
                                    }
                                    
                                    RowLayout {
                                        Button {
                                            id: applyScaleButton
                                            text: "Apply"
                                            onClicked: {
                                                var scaleValue = parseInt(scaleTextField.text)
                                                if (isNaN(scaleValue) || scaleValue < 10 || scaleValue > 500) {
                                                    scaleTextField.text = "100"
                                                    return
                                                }
                                                
                                                var scaleFactor = scaleValue / 100.0
                                                layerImage.manuallyResized = true
                                                
                                                // Get original size (before manual resizing)
                                                var baseWidth, baseHeight
                                                if (layerImage.implicitWidth >= canvasWidth || layerImage.implicitHeight >= canvasHeight) {
                                                    baseWidth = canvasWidth * canvasScale
                                                    baseHeight = canvasHeight * canvasScale
                                                } else {
                                                    baseWidth = layerImage.implicitWidth
                                                    baseHeight = layerImage.implicitHeight
                                                }
                                                
                                                var newWidth = baseWidth * scaleFactor
                                                var newHeight = baseHeight * scaleFactor
                                                
                                                // Don't exceed canvas bounds
                                                var maxWidth = (canvasWidth * canvasScale) - layerItem.x
                                                var maxHeight = (canvasHeight * canvasScale) - layerItem.y
                                                
                                                layerImage.width = Math.min(newWidth, maxWidth)
                                                layerImage.height = Math.min(newHeight, maxHeight)
                                                
                                                console.log("Custom scaled to", scaleValue + "%:", layerImage.width, "x", layerImage.height)
                                                scaleDialog.close()
                                            }
                                        }
                                        
                                        Button {
                                            text: "Cancel"
                                            onClicked: scaleDialog.close()
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // Grid overlay
                    Canvas {
                        id: gridCanvas
                        anchors.fill: parent
                        visible: gridCheckbox.checked
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.strokeStyle = "#333333"
                            ctx.lineWidth = 1
                            
                            var gridSize = 50 * canvasScale
                            
                            // Vertical lines
                            for (var x = gridSize; x < width; x += gridSize) {
                                ctx.beginPath()
                                ctx.moveTo(x, 0)
                                ctx.lineTo(x, height)
                                ctx.stroke()
                            }
                            
                            // Horizontal lines
                            for (var y = gridSize; y < height; y += gridSize) {
                                ctx.beginPath()
                                ctx.moveTo(0, y)
                                ctx.lineTo(width, y)
                                ctx.stroke()
                            }
                        }
                    }
                }
            }
            
            // Playback controls
            GroupBox {
                Layout.fillWidth: true
                title: "Playback"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    
                    // Frame slider
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Frame:"
                            font.pixelSize: 11
                        }
                        
                        Slider {
                            id: frameSlider
                            Layout.fillWidth: true
                            from: 1
                            to: Math.max(1, totalFrames)
                            value: currentFrame
                            stepSize: 1
                            
                            onMoved: {
                                currentFrame = Math.floor(value)
                            }
                            
                            Connections {
                                target: devicePreviewDialog
                                function onCurrentFrameChanged() {
                                    if (!frameSlider.pressed) {
                                        frameSlider.value = currentFrame
                                    }
                                }
                            }
                        }
                        
                        Label {
                            text: currentFrame + "/" + totalFrames
                            font.pixelSize: 11
                            Layout.minimumWidth: 50
                        }
                    }
                    
                    // Control buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Button {
                            contentItem: Row {
                                spacing: 5
                                Image {
                                    source: isPlaying ? "qrc:/BootMod/res/icons/" + (isPlaying ? "pause" : "play") + ".svg" : "qrc:/BootMod/res/icons/play.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: isPlaying ? "Pause" : "Play"
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            onClicked: isPlaying ? pause() : play()
                        }
                        
                        Button {
                            contentItem: Row {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/stop.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: "Stop"
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            onClicked: stop()
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/skip-back.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            onClicked: currentFrame = 1
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/arrow-left.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            enabled: currentFrame > 1
                            onClicked: currentFrame--
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/arrow-right.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            enabled: currentFrame < totalFrames
                            onClicked: currentFrame++
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/skip-forward.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            onClicked: currentFrame = totalFrames
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        CheckBox {
                            id: gridCheckbox
                            text: "Grid"
                            checked: false
                            onCheckedChanged: gridCanvas.requestPaint()
                        }
                    }
                    
                    // Speed and scale controls
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Speed:"
                            font.pixelSize: 11
                        }
                        
                        Slider {
                            Layout.preferredWidth: 120
                            from: 50
                            to: 1000
                            value: playbackSpeed
                            stepSize: 50
                            onValueChanged: playbackSpeed = value
                        }
                        
                        Label {
                            text: playbackSpeed + "ms"
                            font.pixelSize: 11
                            Layout.minimumWidth: 50
                        }
                        
                        Item { width: 20 }
                        
                        Label {
                            text: "Scale:"
                            font.pixelSize: 11
                        }
                        
                        Slider {
                            Layout.preferredWidth: 120
                            from: 0.1
                            to: 2.0
                            value: canvasScale
                            stepSize: 0.1
                            onValueChanged: {
                                canvasScale = value
                                gridCanvas.requestPaint()
                            }
                        }
                        
                        Label {
                            text: (canvasScale * 100).toFixed(0) + "%"
                            font.pixelSize: 11
                            Layout.minimumWidth: 50
                        }
                    }
                }
            }
        }
        
        // Right side - Layer management
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 10
            
            // Layer list
            GroupBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Layers (" + userLayerCount + ")"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 5
                    
                    // Toolbar
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Button {
                            contentItem: RowLayout {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/add.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    Layout.alignment: Qt.AlignVCenter
                                }
                                Text {
                                    text: "Add Image Layer"
                                    color: "#ffffff"
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: "Add a custom image as a new layer"
                            onClicked: {
                                addCustomImageDialog.open()
                            }
                        }
                        
                        Button {
                            contentItem: RowLayout {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/layers.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    Layout.alignment: Qt.AlignVCenter
                                }
                                Text {
                                    text: "Add Logo Layer"
                                    color: "#ffffff"
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: "Add a layer from existing logos"
                            onClicked: {
                                var layer = layerManager.createLayer()
                                layer.logoIndex = currentFrame
                                layer.name = "Logo " + currentFrame
                                layer.xPosition = 50
                                layer.yPosition = 50
                                layer.startFrame = 1
                                layer.endFrame = totalFrames
                                layer.visible = true
                                console.log("Created layer:", layer.name, "logoIndex:", layer.logoIndex, "endFrame:", layer.endFrame)
                                
                                var newIndex = layerManager.layerCount - 1
                                layersList.currentIndex = newIndex
                                selectedLayerIndex = newIndex
                                
                                // Add to undo stack
                                commandManager.executeAddCommand(layerManager, newIndex, layer)
                            }
                        }
                        
                        Button {
                            contentItem: Row {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/delete.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: "Delete"
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            enabled: {
                                if (layersList.currentIndex < 0) return false
                                var layer = layerManager.getLayer(layersList.currentIndex)
                                // Can't delete auto-animation layer (endFrame == -1)
                                return layer && layer.endFrame !== -1
                            }
                            onClicked: {
                                if (layersList.currentIndex >= 0) {
                                    var layer = layerManager.getLayer(layersList.currentIndex)
                                    // Double-check it's not auto-animation
                                    if (layer && layer.endFrame !== -1) {
                                        commandManager.executeRemoveCommand(layerManager, layersList.currentIndex, layer)
                                    }
                                }
                            }
                        }
                        
                        Button {
                            contentItem: RowLayout {
                                spacing: 5
                                Text {
                                    text: "✏️"
                                    font.pixelSize: 16
                                    Layout.alignment: Qt.AlignVCenter
                                }
                                Text {
                                    text: "Edit Current Frame"
                                    color: "#ffffff"
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: "Edit current frame in bitmap editor"
                            onClicked: {
                                // Get the image path for the current frame
                                if (!logoFile.isProjectMode || !logoFile.projectPath) {
                                    console.log("Not in project mode, cannot edit frame")
                                    return
                                }
                                
                                // Get dimensions for the current frame
                                var dims = imageDimensions[currentFrame]
                                if (!dims) {
                                    console.log("No dimensions found for frame", currentFrame)
                                    return
                                }
                                
                                // Use the same path format as the image display: /images/logo_{frame}_{width}x{height}.png
                                var framePath = logoFile.projectPath + "/images/logo_" + currentFrame + "_" + dims.width + "x" + dims.height + ".png"
                                console.log("Opening bitmap editor for frame:", framePath)
                                bitmapEditor.openEditor(framePath)
                            }
                        }
                        
                        MenuSeparator {
                            Layout.preferredHeight: 20
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/arrow-up.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            ToolTip.text: "Move Layer Up"
                            ToolTip.visible: hovered
                            enabled: {
                                if (layersList.currentIndex < 0) return false
                                var layer = layerManager.getLayer(layersList.currentIndex)
                                if (!layer || layer.endFrame === -1) return false
                                
                                // Can move up if not already at top (index 0 or right after auto-animation)
                                // Find first non-auto-animation layer
                                var firstUserLayerIndex = -1
                                for (var i = 0; i < layerManager.layerCount; i++) {
                                    var l = layerManager.getLayer(i)
                                    if (l && l.endFrame !== -1) {
                                        firstUserLayerIndex = i
                                        break
                                    }
                                }
                                
                                return layersList.currentIndex > firstUserLayerIndex
                            }
                            onClicked: {
                                if (layersList.currentIndex > 0) {
                                    layerManager.moveLayer(layersList.currentIndex, layersList.currentIndex - 1)
                                    layersList.currentIndex = layersList.currentIndex - 1
                                    selectedLayerIndex = layersList.currentIndex
                                }
                            }
                        }
                        
                        Button {
                            contentItem: Item {
                                implicitWidth: 24
                                implicitHeight: 24
                                Image {
                                    source: "qrc:/BootMod/res/icons/arrow-down.svg"
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            ToolTip.text: "Move Layer Down"
                            ToolTip.visible: hovered
                            enabled: {
                                if (layersList.currentIndex < 0) return false
                                var layer = layerManager.getLayer(layersList.currentIndex)
                                if (!layer || layer.endFrame === -1) return false
                                
                                // Can move down if not already at bottom
                                return layersList.currentIndex < layerManager.layerCount - 1
                            }
                            onClicked: {
                                if (layersList.currentIndex < layerManager.layerCount - 1) {
                                    layerManager.moveLayer(layersList.currentIndex, layersList.currentIndex + 1)
                                    layersList.currentIndex = layersList.currentIndex + 1
                                    selectedLayerIndex = layersList.currentIndex
                                }
                            }
                        }
                        
                        Button {
                            contentItem: Row {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/merge-down.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: "Merge Down"
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            enabled: {
                                if (layersList.currentIndex < 0) return false
                                
                                // Get current layer
                                var currentLayer = layerManager.getLayer(layersList.currentIndex)
                                if (!currentLayer || currentLayer.endFrame === -1) return false
                                
                                // Check if there's a layer below (higher index, since layers are rendered bottom-up)
                                var nextVisibleIndex = -1
                                for (var i = layersList.currentIndex + 1; i < layerManager.layerCount; i++) {
                                    var layer = layerManager.getLayer(i)
                                    if (layer && layer.endFrame !== -1) {
                                        nextVisibleIndex = i
                                        break
                                    }
                                }
                                
                                return nextVisibleIndex !== -1
                            }
                            onClicked: {
                                if (layersList.currentIndex >= 0) {
                                    // Find next visible layer (not auto-animation)
                                    var nextVisibleIndex = -1
                                    for (var i = layersList.currentIndex + 1; i < layerManager.layerCount; i++) {
                                        var layer = layerManager.getLayer(i)
                                        if (layer && layer.endFrame !== -1) {
                                            nextVisibleIndex = i
                                            break
                                        }
                                    }
                                    
                                    if (nextVisibleIndex !== -1) {
                                        mergeLayers(layersList.currentIndex, nextVisibleIndex)
                                    }
                                }
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        Button {
                            contentItem: Row {
                                spacing: 5
                                Image {
                                    source: "qrc:/BootMod/res/icons/save.svg"
                                    width: 16
                                    height: 16
                                    sourceSize: Qt.size(16, 16)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: "Save"
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            onClicked: saveLayers()
                        }
                    }
                    
                    // Layer list
                    ListView {
                        id: layersList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: layerManager.layerCount
                        spacing: 2
                        
                        delegate: Rectangle {
                            // Hide auto-animation layer from list
                            visible: {
                                if (!layerData) return false
                                // Hide if it's the auto-animation layer (endFrame == -1)
                                return layerData.endFrame !== -1
                            }
                            height: visible ? 70 : 0
                            
                            width: layersList.width
                            color: layersList.currentIndex === index ? "#3d5a80" : "#293241"
                            border.color: "#4a5568"
                            border.width: 1
                            radius: 4
                            
                            property var layerData: layerManager.getLayer(index)
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    layersList.currentIndex = index
                                    selectedLayerIndex = index
                                }
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8
                                
                                Button {
                                    Layout.preferredWidth: 30
                                    Layout.preferredHeight: 30
                                    contentItem: Item {
                                        implicitWidth: 24
                                        implicitHeight: 24
                                        Image {
                                            source: layerData && layerData.visible ? "qrc:/BootMod/res/icons/eye.svg" : "qrc:/BootMod/res/icons/eye-off.svg"
                                            anchors.centerIn: parent
                                            width: 16
                                            height: 16
                                            sourceSize: Qt.size(16, 16)
                                            fillMode: Image.PreserveAspectFit
                                        }
                                    }
                                    onClicked: {
                                        if (layerData) {
                                            layerData.visible = !layerData.visible
                                        }
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    
                                    TextField {
                                        Layout.fillWidth: true
                                        text: layerData ? layerData.name : ""
                                        font.pixelSize: 12
                                        onEditingFinished: {
                                            if (layerData) {
                                                layerData.name = text
                                            }
                                        }
                                    }
                                    
                                    Label {
                                        text: layerData ? "Logo #" + layerData.logoIndex + " | " + 
                                                         layerData.xPosition + "," + layerData.yPosition : ""
                                        font.pixelSize: 10
                                        color: "#9ca3af"
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Layer properties
            GroupBox {
                Layout.fillWidth: true
                title: "Properties"
                visible: {
                    if (layersList.currentIndex < 0) return false
                    var layer = layerManager.getLayer(layersList.currentIndex)
                    // Don't show properties for auto-animation layer
                    return layer && layer.endFrame !== -1
                }
                
                property var currentLayer: {
                    if (layersList.currentIndex >= 0) {
                        var layer = layerManager.getLayer(layersList.currentIndex)
                        // Only return layer if it's not auto-animation
                        if (layer && layer.endFrame !== -1) {
                            return layer
                        }
                    }
                    return null
                }
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    // Update all controls when layer selection changes
                    Connections {
                        target: layersList
                        function onCurrentIndexChanged() {
                            // Force UI update by reading from currentLayer
                            var layer = parent.parent.parent.currentLayer
                            if (!layer) return  // Safety check
                            
                            logoIndexSpinBox.value = layer.logoIndex
                            xPositionSpinBox.value = layer.xPosition
                            yPositionSpinBox.value = layer.yPosition
                            opacitySlider.value = layer.opacity
                            startFrameSpinBox.value = layer.startFrame
                            endFrameSpinBox.value = layer.endFrame
                        }
                    }
                    
                    // Logo index
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Logo:"
                            Layout.preferredWidth: 70
                        }
                        
                        SpinBox {
                            id: logoIndexSpinBox
                            Layout.fillWidth: true
                            from: 1
                            to: totalFrames
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.logoIndex : 1
                            onValueModified: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    parent.parent.parent.parent.currentLayer.logoIndex = value
                                }
                            }
                        }
                    }
                    
                    // Position
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Position:"
                            Layout.preferredWidth: 70
                        }
                        
                        Label { text: "X:" }
                        SpinBox {
                            id: xPositionSpinBox
                            from: -2000
                            to: 4000
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.xPosition : 0
                            editable: true
                            
                            property int previousValue: 0
                            
                            onValueChanged: {
                                // Track previous value
                                if (parent.parent.parent.parent.currentLayer) {
                                    previousValue = parent.parent.parent.parent.currentLayer.xPosition
                                }
                            }
                            
                            onValueModified: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    var oldX = previousValue
                                    var newX = value
                                    var y = parent.parent.parent.parent.currentLayer.yPosition
                                    
                                    // Update the actual value
                                    parent.parent.parent.parent.currentLayer.xPosition = value
                                    
                                    // Create undo command
                                    if (oldX !== newX) {
                                        commandManager.executeMoveCommand(
                                            layerManager, 
                                            selectedLayerIndex, 
                                            Qt.point(oldX, y), 
                                            Qt.point(newX, y)
                                        )
                                    }
                                }
                            }
                        }
                        
                        Label { text: "Y:" }
                        SpinBox {
                            id: yPositionSpinBox
                            from: -2000
                            to: 4000
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.yPosition : 0
                            editable: true
                            
                            property int previousValue: 0
                            
                            onValueChanged: {
                                // Track previous value
                                if (parent.parent.parent.parent.currentLayer) {
                                    previousValue = parent.parent.parent.parent.currentLayer.yPosition
                                }
                            }
                            
                            onValueModified: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    var x = parent.parent.parent.parent.currentLayer.xPosition
                                    var oldY = previousValue
                                    var newY = value
                                    
                                    // Update the actual value
                                    parent.parent.parent.parent.currentLayer.yPosition = value
                                    
                                    // Create undo command
                                    if (oldY !== newY) {
                                        commandManager.executeMoveCommand(
                                            layerManager, 
                                            selectedLayerIndex, 
                                            Qt.point(x, oldY), 
                                            Qt.point(x, newY)
                                        )
                                    }
                                }
                            }
                        }
                    }
                    
                    // Opacity
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Opacity:"
                            Layout.preferredWidth: 70
                        }
                        
                        Slider {
                            id: opacitySlider
                            Layout.fillWidth: true
                            from: 0.0
                            to: 1.0
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.opacity : 1.0
                            stepSize: 0.05
                            
                            property real startOpacity: 1.0
                            
                            onPressedChanged: {
                                if (pressed) {
                                    // Store starting opacity for undo
                                    startOpacity = value
                                } else {
                                    // Create undo command when released if value changed
                                    if (parent.parent.parent.parent.currentLayer && Math.abs(value - startOpacity) > 0.001) {
                                        commandManager.executeOpacityCommand(
                                            layerManager,
                                            selectedLayerIndex,
                                            startOpacity,
                                            value
                                        )
                                    }
                                }
                            }
                            
                            onMoved: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    parent.parent.parent.parent.currentLayer.opacity = value
                                }
                            }
                        }
                        
                        Label {
                            text: parent.parent.parent.parent.currentLayer 
                                  ? (parent.parent.parent.parent.currentLayer.opacity * 100).toFixed(0) + "%" 
                                  : "100%"
                            Layout.minimumWidth: 45
                        }
                    }
                    
                    // Frame range
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "Start:"
                            Layout.preferredWidth: 70
                        }
                        
                        SpinBox {
                            id: startFrameSpinBox
                            Layout.fillWidth: true
                            from: 1
                            to: Math.max(1, totalFrames)
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.startFrame : 1
                            onValueModified: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    parent.parent.parent.parent.currentLayer.startFrame = value
                                }
                            }
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "End:"
                            Layout.preferredWidth: 70
                        }
                        
                        SpinBox {
                            id: endFrameSpinBox
                            Layout.fillWidth: true
                            from: -1
                            to: Math.max(1, totalFrames)
                            value: parent.parent.parent.parent.currentLayer ? parent.parent.parent.parent.currentLayer.endFrame : -1
                            onValueModified: {
                                if (parent.parent.parent.parent.currentLayer) {
                                    parent.parent.parent.parent.currentLayer.endFrame = value
                                }
                            }
                            
                            textFromValue: function(value) {
                                return value === -1 ? "All" : value.toString()
                            }
                        }
                    }
                }  // End ColumnLayout (properties)
            }  // End GroupBox (properties)
        }  // End ColumnLayout (right side)
        }  // End RowLayout (main content)
    }  // End Item (main content wrapper)
    
    // Close button in the window
    ToolBar {
        id: windowToolbar
        anchors.bottom: parent.bottom
        width: parent.width
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            
            // Undo/Redo buttons
            Button {
                text: "↶ Undo"
                enabled: commandManager.canUndo
                ToolTip.text: commandManager.canUndo ? commandManager.undoText : "Nothing to undo"
                ToolTip.visible: hovered
                onClicked: commandManager.undo()
            }
            
            Button {
                text: "↷ Redo"
                enabled: commandManager.canRedo
                ToolTip.text: commandManager.canRedo ? commandManager.redoText : "Nothing to redo"
                ToolTip.visible: hovered
                onClicked: commandManager.redo()
            }
            
            Rectangle {
                width: 1
                height: parent.height * 0.6
                color: "#cccccc"
                Layout.leftMargin: 5
                Layout.rightMargin: 5
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: "Close"
                onClicked: {
                    stop()
                    saveLayers()
                    devicePreviewDialog.close()
                }
            }
        }
    }
    
    // Keyboard shortcuts for undo/redo
    Shortcut {
        sequence: "Ctrl+Z"
        enabled: commandManager.canUndo
        onActivated: commandManager.undo()
    }
    
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        enabled: commandManager.canRedo
        onActivated: commandManager.redo()
    }
    
    Shortcut {
        sequence: "Ctrl+Y"
        enabled: commandManager.canRedo
        onActivated: commandManager.redo()
    }
    
    onClosing: {
        stop()
        saveLayers()
    }
    
    // File dialog for adding custom images
    FileDialog {
        id: addCustomImageDialog
        title: "Add Custom Image Layer"
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.bmp)", "All files (*)"]
        fileMode: FileDialog.OpenFile
        
        onAccepted: {
            var imagePath = selectedFile.toString()
            // Remove file:// prefix
            if (imagePath.startsWith("file://")) {
                imagePath = imagePath.substring(7)
            }
            
            console.log("Adding custom image layer from:", imagePath)
            
            // Create a new layer with the custom image
            var layer = layerManager.createLayer()
            layer.name = "Custom Image"
            layer.logoIndex = 0  // 0 means custom image, not from logo.bin
            layer.customImagePath = imagePath
            layer.xPosition = 100
            layer.yPosition = 100
            layer.startFrame = 1
            layer.endFrame = totalFrames
            layer.visible = true
            layer.opacity = 1.0
            
            // Select the new layer
            var newIndex = layerManager.layerCount - 1
            layersList.currentIndex = newIndex
            selectedLayerIndex = newIndex
            
            // Add to undo stack
            commandManager.executeAddCommand(layerManager, newIndex, layer)
            
            console.log("Created custom image layer:", layer.name, "path:", layer.customImagePath)
        }
    }
}  // End ApplicationWindow