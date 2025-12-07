cmake_minimum_required(VERSION 3.16)

# Get version from environment or use default
if(DEFINED ENV{BOOTMOD_VERSION})
    set(PROJECT_VERSION $ENV{BOOTMOD_VERSION})
else()
    set(PROJECT_VERSION "2.0.0")
endif()

project(bootmod-gui VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Pass version to source code
add_compile_definitions(BOOTMOD_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Widgets)

# Find dependencies
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

# lodepng library
set(LODEPNG_DIR ${CMAKE_SOURCE_DIR}/../external/lodepng)
add_library(lodepng STATIC ${LODEPNG_DIR}/lodepng.cpp)
target_include_directories(lodepng PUBLIC ${LODEPNG_DIR})

# Source files
set(SOURCES
    src/main.cpp
    src/logofile.cpp
    src/logolistmodel.cpp
    src/thumbnailprovider.cpp
    src/appcontext.cpp
    src/logolayer.cpp
    ../src/bootmod.cpp
    ../src/bootmod_png.cpp
    ../src/splash.cpp
)

set(HEADERS
    src/logofile.h
    src/logolistmodel.h
    src/thumbnailprovider.h
    src/appcontext.h
    src/logolayer.h
    ../include/bootmod.h
    ../include/splash.h
)

# QML files
set(QML_FILES
    qml/Main.qml
    qml/AboutDialog.qml
    qml/DevicePreviewDialog.qml
)

# Resource files
set(RESOURCE_FILES
    res/bootmod_logo.svg
    res/icons/play.svg
    res/icons/pause.svg
    res/icons/stop.svg
    res/icons/skip-back.svg
    res/icons/skip-forward.svg
    res/icons/layers.svg
    res/icons/add.svg
    res/icons/delete.svg
    res/icons/move.svg
    res/icons/eye.svg
    res/icons/eye-off.svg
    res/icons/save.svg
    res/icons/merge-down.svg
    res/icons/phone.svg
    res/icons/chart.svg
    res/icons/package.svg
    res/icons/folder.svg
    res/icons/folder-open.svg
    res/icons/file.svg
    res/icons/link.svg
    res/icons/telegram.svg
    res/icons/heart.svg
    res/icons/sparkles.svg
    res/icons/user.svg
    res/icons/upload.svg
    res/icons/download.svg
    res/icons/close.svg
    res/icons/arrow-up.svg
    res/icons/arrow-down.svg
    res/icons/arrow-left.svg
    res/icons/arrow-right.svg
)

# Create executable
qt6_add_executable(bootmod-gui ${SOURCES} ${HEADERS})

# Add QML module
qt6_add_qml_module(bootmod-gui
    URI BootMod
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCES ${RESOURCE_FILES}
)

# Include directories
target_include_directories(bootmod-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${LODEPNG_DIR}
)

# Link libraries
target_link_libraries(bootmod-gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    ZLIB::ZLIB
    PNG::PNG
    lodepng
)

# Install
install(TARGETS bootmod-gui
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt6_finalize_target(bootmod-gui)
