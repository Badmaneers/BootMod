cmake_minimum_required(VERSION 3.16)

project(BitmapEditorLib VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

# TOOL
set(QBE_TOOL
    tool/pencil.h
    tool/pencil.cpp
    tool/eraser.h
    tool/eraser.cpp
    tool/brush.h
    tool/brush.cpp
    tool/fillcolor.h
    tool/fillcolor.cpp
    tool/eyedropper.h
    tool/eyedropper.cpp
    tool/text.h
    tool/text.cpp
)

# LAYER - Rename to avoid conflict with BootMod's LayerManager
set(QBE_LAYER
    layer/bitmaplayer.h
    layer/bitmaplayer.cpp
    layer/textlayer.h
    layer/textlayer.cpp
    layer/imagelayer.h
    layer/imagelayer.cpp
)

# UTILITY
set(QBE_UTILITY
    utility/mouseeventhelper.h
    utility/mouseeventhelper.cpp
    utility/colorpicker.h
    utility/colorpicker.cpp
    utility/gradientwidget.h
    utility/gradientwidget.cpp
    utility/coloritem.h
    utility/coloritem.cpp
    utility/fontselector.h
    utility/fontselector.cpp
)

# UI
set(QBE_UI
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    projectwizard.h
    projectwizard.cpp
    projectwizard.ui
    newproject.h
    newproject.cpp
    newproject.ui
    openproject.h
    openproject.cpp
    openproject.ui
    projectfromimage.h
    projectfromimage.cpp
    projectfromimage.ui
    importimage.h
    importimage.cpp
    importimage.ui
    exportproject.h
    exportproject.cpp
    exportproject.ui
    about.h
    about.cpp
    about.ui
)

# RESOURCES
set(QBE_RES
    icons.qrc
    theme.qrc
    splash.qrc
)

# BASE
set(QBE_BASE
    base/bitmapappcontext.h
    base/bitmapappcontext.cpp
    base/module.h
    base/config.h
    base/dialogs.h
    base/project.h
    base/project.cpp
    base/layer.h
    base/layer.cpp
    base/bitmaplayermanager.h
    base/bitmaplayermanager.cpp
    base/toolcontroller.h
    base/toolcontroller.cpp
    base/workspace.h
    base/workspace.cpp
    base/tool.h
    base/tool.cpp
)

# Build as a static library (no main.cpp)
add_library(BitmapEditorLib STATIC
    ${QBE_BASE}
    ${QBE_LAYER}
    ${QBE_TOOL}
    ${QBE_UTILITY}
    ${QBE_UI}
    ${QBE_RES}
)

target_link_libraries(BitmapEditorLib PUBLIC 
    Qt6::Widgets
    Qt6::PrintSupport
)

target_include_directories(BitmapEditorLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/base
    ${CMAKE_CURRENT_SOURCE_DIR}/layer
    ${CMAKE_CURRENT_SOURCE_DIR}/tool
    ${CMAKE_CURRENT_SOURCE_DIR}/utility
)
