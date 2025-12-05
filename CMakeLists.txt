cmake_minimum_required(VERSION 3.10)
project(bootmod VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/bootmod.cpp
    src/bootmod_png.cpp
    src/main.cpp
)

# Header files
set(HEADERS
    include/bootmod.h
    include/version.h
)

# Create executable
add_executable(bootmod ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(bootmod PRIVATE
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(bootmod
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bootmod PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Installation
install(TARGETS bootmod
    RUNTIME DESTINATION bin
)

# Create library version (optional)
add_library(bootmod_lib STATIC
    src/bootmod.cpp
    src/bootmod_png.cpp
)

target_include_directories(bootmod_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(bootmod_lib PUBLIC
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

install(TARGETS bootmod_lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include
)
