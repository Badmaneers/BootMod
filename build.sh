#!/bin/bash

# BootMod Build Script
# Builds both CLI and GUI applications with version management

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Default version
VERSION="1.1.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Build options
BUILD_CLI=true
BUILD_GUI=true
CLEAN=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --cli-only)
            BUILD_GUI=false
            shift
            ;;
        --gui-only)
            BUILD_CLI=false
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --version=*)
            VERSION="${1#*=}"
            shift
            ;;
        --help)
            echo "BootMod Build Script"
            echo ""
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --version=X.Y.Z      Specify version number (default: 1.6.0)"
            echo "  --cli-only           Build only CLI tool"
            echo "  --gui-only           Build only GUI application"
            echo "  --clean              Clean build directories before building"
            echo "  --help               Show this help message"
            echo ""
            echo "Examples:"
            echo "  ./build.sh                    # Build both CLI and GUI"
            echo "  ./build.sh --version=1.7.0    # Build with version 1.7.0"
            echo "  ./build.sh --cli-only         # Build only CLI"
            echo "  ./build.sh --gui-only         # Build only GUI"
            echo "  ./build.sh --clean            # Clean and build"
            exit 0
            ;;
        [0-9]*.[0-9]*.[0-9]*)
            # Allow version as first positional argument
            VERSION="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo ""
echo -e "${BLUE}╔═══════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║          BootMod Build System v${VERSION}        ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════╝${NC}"
echo ""

echo ""

# Update version in project files
echo -e "${CYAN}Updating version to ${VERSION}...${NC}"

# Update CLI Makefile
if [ -f "Makefile" ]; then
    sed -i "s/VERSION = .*/VERSION = ${VERSION}/" Makefile
    echo -e "${GREEN}✓${NC} Updated Makefile"
fi

# Update GUI CMakeLists.txt
if [ -f "gui/CMakeLists.txt" ]; then
    sed -i "s/set(PROJECT_VERSION \"[0-9.]*\")/set(PROJECT_VERSION \"${VERSION}\")/" gui/CMakeLists.txt
    echo -e "${GREEN}✓${NC} Updated gui/CMakeLists.txt"
fi

echo ""

# Check for dependencies
echo -e "${CYAN}Checking dependencies...${NC}"

if ! command -v g++ &> /dev/null; then
    echo -e "${RED}Error: g++ not found. Please install build-essential.${NC}"
    exit 1
fi

if ! ldconfig -p | grep -q libz.so; then
    echo -e "${RED}Error: zlib not found. Please install zlib1g-dev.${NC}"
    exit 1
fi

if ! ldconfig -p | grep -q libpng; then
    echo -e "${RED}Error: libpng not found. Please install libpng-dev.${NC}"
    exit 1
fi

if $BUILD_GUI; then
    if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
        echo -e "${YELLOW}Warning: Qt6 qmake not found. GUI build may fail.${NC}"
        echo -e "${YELLOW}Install with: sudo apt install qt6-base-dev qt6-declarative-dev${NC}"
    fi
fi

echo -e "${GREEN}✓${NC} All dependencies found!"
echo ""

# Clean if requested
if $CLEAN; then
    echo -e "${CYAN}Cleaning build directories...${NC}"
    make clean 2>/dev/null || true
    if [ -d "gui/build" ]; then
        rm -rf gui/build
        echo -e "${GREEN}✓${NC} Cleaned gui/build"
    fi
    echo ""
fi

# Build CLI
if $BUILD_CLI; then
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo -e "${BLUE}         Building CLI Tool              ${NC}"
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo ""
    
    make -j$(nproc)
    
    if [ ! -f bin/bootmod ]; then
        echo -e "${RED}Error: CLI build failed - binary not found${NC}"
        exit 1
    fi
    
    echo ""
    echo -e "${GREEN}✓ CLI build successful!${NC}"
    echo -e "  Binary: ${CYAN}bin/mtklogo${NC}"
    echo ""
fi

# Build GUI
if $BUILD_GUI; then
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo -e "${BLUE}       Building GUI Application         ${NC}"
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo ""
    
    # Export version for CMake
    export BOOTMOD_VERSION="${VERSION}"
    
    mkdir -p gui/build
    cd gui/build
    
    echo -e "${CYAN}Running CMake...${NC}"
    cmake .. || {
        echo -e "${RED}Error: CMake configuration failed${NC}"
        exit 1
    }
    
    echo ""
    echo -e "${CYAN}Compiling...${NC}"
    make -j$(nproc) || {
        echo -e "${RED}Error: GUI build failed${NC}"
        exit 1
    }
    
    cd ../..
    
    if [ ! -f gui/build/bootmod-gui ]; then
        echo -e "${RED}Error: GUI build failed - binary not found${NC}"
        exit 1
    fi
    
    echo ""
    echo -e "${GREEN}✓ GUI build successful!${NC}"
    echo -e "  Binary: ${CYAN}gui/build/bootmod-gui${NC}"
    echo ""
fi

# Summary
echo ""
echo -e "${BLUE}╔═══════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║          Build Summary                    ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════╝${NC}"
echo ""
echo -e "${GREEN}Version:${NC} ${VERSION}"
echo ""

if $BUILD_CLI; then
    echo -e "${GREEN}CLI Tool:${NC}"
    echo -e "  Location: ${CYAN}bin/mtklogo${NC}"
    echo -e "  Usage:    ${CYAN}./bin/mtklogo help${NC}"
    echo ""
fi

if $BUILD_GUI; then
    echo -e "${GREEN}GUI Application:${NC}"
    echo -e "  Location: ${CYAN}gui/build/bootmod-gui${NC}"
    echo -e "  Usage:    ${CYAN}./gui/build/bootmod-gui${NC}"
    echo ""
fi

echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
echo ""
