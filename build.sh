#!/bin/bash

# BootMod Build Script
# Complete setup, build, and release packaging system

set -e  # Exit on error

# Script version
SCRIPT_VERSION="1.0.0"
PROJECT_VERSION="1.0.0"

# Build options
BUILD_MODE="Debug"
BUILD_CLI=true
BUILD_GUI=true
CREATE_RELEASE=false
CLEAN_BUILD=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Print functions
print_header() {
    echo -e "${BLUE}╔═══════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║     BootMod Build System v${SCRIPT_VERSION}         ║${NC}"
    echo -e "${BLUE}║     Project Version: ${PROJECT_VERSION}              ║${NC}"
    echo -e "${BLUE}║     Build Mode: ${BUILD_MODE}                    ║${NC}"
    echo -e "${BLUE}╚═══════════════════════════════════════════╝${NC}"
    echo ""
}

print_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Options:
  -h, --help          Show this help message
  -r, --release       Build in release mode with optimizations
  -c, --clean         Clean build directory before building
  -p, --package       Create release package (tar.gz)
  -v, --version VER   Set project version (default: ${PROJECT_VERSION})
  --cli-only          Build only CLI tool
  --gui-only          Build only GUI application
  
Build Modes:
  Debug (default)     Build with debug symbols
  Release (-r)        Build optimized for distribution
  
Examples:
  $0                          # Debug build (CLI + GUI)
  $0 --release                # Release build
  $0 --release --package      # Release build + create package
  $0 --clean --release        # Clean + Release build
  $0 --cli-only               # Build only CLI tool
  $0 --gui-only               # Build only GUI
  $0 --version 2.0.0 --release --package  # Version 2.0.0 release

EOF
    exit 0
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

print_step() {
    echo -e "${BLUE}→${NC} $1"
}

print_section() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                ;;
            -r|--release)
                BUILD_MODE="Release"
                shift
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -p|--package)
                CREATE_RELEASE=true
                BUILD_MODE="Release"  # Force release mode for packaging
                shift
                ;;
            -v|--version)
                PROJECT_VERSION="$2"
                shift 2
                ;;
            --cli-only)
                BUILD_GUI=false
                shift
                ;;
            --gui-only)
                BUILD_CLI=false
                shift
                ;;
            *)
                echo -e "${RED}Unknown option: $1${NC}"
                print_usage
                ;;
        esac
    done
}

# Update version in project files
update_version() {
    print_section "Updating Project Version"
    
    print_step "Setting version to ${PROJECT_VERSION}..."
    
    # Update CLI Makefile if it exists
    if [ -f "Makefile" ] && [ "$BUILD_CLI" = true ]; then
        sed -i "s/VERSION = .*/VERSION = ${PROJECT_VERSION}/" Makefile 2>/dev/null || true
        print_success "Updated CLI Makefile"
    fi
    
    # Update GUI CMakeLists.txt
    if [ -f "gui/CMakeLists.txt" ] && [ "$BUILD_GUI" = true ]; then
        sed -i "s/set(PROJECT_VERSION \"[0-9.]*\")/set(PROJECT_VERSION \"${PROJECT_VERSION}\")/" gui/CMakeLists.txt 2>/dev/null || true
        sed -i "s/project(bootmod-gui VERSION [0-9.]*)/project(bootmod-gui VERSION ${PROJECT_VERSION})/" gui/CMakeLists.txt 2>/dev/null || true
        print_success "Updated GUI CMakeLists.txt"
    fi
    
    print_success "Version updated to ${PROJECT_VERSION}"
}

# Check dependencies
check_dependencies() {
    print_section "Checking Dependencies"
    
    local MISSING_DEPS=()
    
    # Common dependencies
    if ! command_exists g++; then
        MISSING_DEPS+=("g++")
    fi
    
    if ! command_exists make; then
        MISSING_DEPS+=("make")
    fi
    
    # CLI dependencies
    if [ "$BUILD_CLI" = true ]; then
        if ! ldconfig -p 2>/dev/null | grep -q libz.so; then
            MISSING_DEPS+=("zlib")
        fi
        
        if ! ldconfig -p 2>/dev/null | grep -q libpng; then
            MISSING_DEPS+=("libpng")
        fi
    fi
    
    # GUI dependencies
    if [ "$BUILD_GUI" = true ]; then
        if ! command_exists cmake; then
            MISSING_DEPS+=("cmake")
        fi
        
        # Check for Qt6
        if ! pkg-config --exists Qt6Core 2>/dev/null && ! command_exists qmake6; then
            print_info "Qt6 not found - GUI build may fail"
            print_info "Install: sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev"
        fi
    fi
    
    if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${MISSING_DEPS[*]}"
        echo ""
        print_info "Install commands:"
        echo ""
        echo "  Ubuntu/Debian:"
        echo "    sudo apt update"
        echo "    sudo apt install build-essential cmake zlib1g-dev libpng-dev"
        echo "    sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev"
        echo ""
        echo "  Arch Linux:"
        echo "    sudo pacman -S base-devel cmake zlib libpng qt6-base qt6-declarative"
        echo ""
        echo "  Fedora:"
        echo "    sudo dnf install gcc-c++ cmake zlib-devel libpng-devel qt6-qtbase-devel"
        echo ""
        exit 1
    fi
    
    print_success "All dependencies satisfied"
    
    # Print what will be built
    echo ""
    if [ "$BUILD_CLI" = true ] && [ "$BUILD_GUI" = true ]; then
        print_info "Building: CLI + GUI"
    elif [ "$BUILD_CLI" = true ]; then
        print_info "Building: CLI only"
    elif [ "$BUILD_GUI" = true ]; then
        print_info "Building: GUI only"
    fi
}

# Build CLI tool
build_cli() {
    if [ "$BUILD_CLI" = false ]; then
        return 0
    fi
    
    print_section "Building CLI Tool"
    
    print_step "Compiling MTK Logo Tool..."
    
    CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    print_info "Using ${CORES} CPU cores"
    
    if [ "$BUILD_MODE" = "Release" ]; then
        print_info "Release mode: Enabling optimizations"
        make CFLAGS="-O3 -march=native" -j$CORES 2>&1 | tee build_cli.log || true
    else
        make -j$CORES 2>&1 | tee build_cli.log || true
    fi
    
    if [ ! -f "bin/mtklogo" ]; then
        print_error "CLI build failed - binary not found"
        print_info "Check build_cli.log for details"
        exit 1
    fi
    
    CLI_SIZE=$(du -h bin/mtklogo | cut -f1)
    print_success "CLI tool built successfully (${CLI_SIZE})"
    
    if [ "$BUILD_MODE" = "Release" ]; then
        print_step "Stripping debug symbols..."
        strip bin/mtklogo
        STRIPPED_SIZE=$(du -h bin/mtklogo | cut -f1)
        print_success "Binary stripped (${STRIPPED_SIZE})"
    fi
}

# Build GUI application
build_gui() {
    if [ "$BUILD_GUI" = false ]; then
        return 0
    fi
    
    print_section "Building GUI Application"
    
    print_step "Creating build directory..."
    mkdir -p gui/build
    cd gui/build
    
    print_step "Configuring CMake (${BUILD_MODE} mode)..."
    
    if [ "$BUILD_MODE" = "Release" ]; then
        print_info "Enabling compiler optimizations (-O3)"
        cmake .. -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG" \
              -DCMAKE_INSTALL_PREFIX=/usr 2>&1 | tee ../../cmake.log || { cd ../..; exit 1; }
    else
        cmake .. -DCMAKE_BUILD_TYPE=Debug 2>&1 | tee ../../cmake.log || { cd ../..; exit 1; }
    fi
    print_success "CMake configured"
    
    print_step "Compiling GUI application..."
    CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    print_info "Using ${CORES} CPU cores"
    
    make -j$CORES 2>&1 | tee ../../build_gui.log || { cd ../..; exit 1; }
    print_success "Compilation complete"
    
    cd ../..
    
    if [ ! -f "gui/build/bootmod-gui" ]; then
        print_error "GUI build failed - binary not found"
        exit 1
    fi
    
    GUI_SIZE=$(du -h gui/build/bootmod-gui | cut -f1)
    print_success "GUI application built successfully (${GUI_SIZE})"
    
    if [ "$BUILD_MODE" = "Release" ]; then
        print_step "Stripping debug symbols..."
        strip gui/build/bootmod-gui
        STRIPPED_SIZE=$(du -h gui/build/bootmod-gui | cut -f1)
        print_success "Binary stripped (${STRIPPED_SIZE})"
    fi
}

# Create release package
create_release_package() {
    print_section "Creating Release Package"
    
    local RELEASE_NAME="bootmod-v${PROJECT_VERSION}-$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m)"
    local PACKAGE_NAME="${RELEASE_NAME}.tar.gz"
    
    print_step "Preparing release directory..."
    rm -rf "dist/${RELEASE_NAME}"
    mkdir -p "dist/${RELEASE_NAME}/bin"
    
    # Copy CLI binary
    if [ "$BUILD_CLI" = true ] && [ -f "bin/mtklogo" ]; then
        print_step "Copying CLI binary..."
        cp bin/mtklogo "dist/${RELEASE_NAME}/bin/"
        print_success "CLI binary included"
    fi
    
    # Copy GUI binary and resources
    local HAS_GUI=false
    if [ "$BUILD_GUI" = true ] && [ -f "gui/build/bootmod-gui" ]; then
        print_step "Copying GUI binary..."
        cp gui/build/bootmod-gui "dist/${RELEASE_NAME}/bin/"
        HAS_GUI=true
        
        # Copy resources
        if [ -d "gui/res" ]; then
            cp -r gui/res "dist/${RELEASE_NAME}/"
            print_success "GUI resources included"
        fi
        
        if [ -d "gui/qml" ]; then
            cp -r gui/qml "dist/${RELEASE_NAME}/"
            print_success "QML files included"
        fi
        
        print_success "GUI binary included"
    fi
    
    # Copy documentation
    if [ -f "README.md" ]; then
        cp README.md "dist/${RELEASE_NAME}/"
        print_success "README.md included"
    fi
    
    if [ -f "LICENSE" ]; then
        cp LICENSE "dist/${RELEASE_NAME}/"
        print_success "LICENSE included"
    fi
    
    # Create run.sh launcher for GUI
    if [ "$HAS_GUI" = true ]; then
        print_step "Creating run.sh launcher..."
        cat > "dist/${RELEASE_NAME}/run.sh" << 'LAUNCHER_EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${SCRIPT_DIR}:${LD_LIBRARY_PATH}"
exec "${SCRIPT_DIR}/bin/bootmod-gui" "$@"
LAUNCHER_EOF
        chmod +x "dist/${RELEASE_NAME}/run.sh"
        print_success "run.sh launcher created"
    fi
    
    # Make binaries executable
    chmod +x "dist/${RELEASE_NAME}/bin/"* 2>/dev/null || true
    
    # Create tarball
    print_step "Creating tarball..."
    cd dist
    tar -czf "${PACKAGE_NAME}" "${RELEASE_NAME}"
    
    # Calculate checksums
    sha256sum "${PACKAGE_NAME}" > "${PACKAGE_NAME}.sha256"
    md5sum "${PACKAGE_NAME}" > "${PACKAGE_NAME}.md5"
    cd ..
    
    print_success "Release package created: dist/${PACKAGE_NAME}"
    print_info "Package size: $(du -h dist/${PACKAGE_NAME} | cut -f1)"
    echo ""
    cat "dist/${PACKAGE_NAME}.sha256"
    echo ""
}

# Main function
main() {
    parse_args "$@"
    print_header
    update_version
    
    if [ "$CLEAN_BUILD" = true ]; then
        print_section "Cleaning Build Directories"
        make clean 2>/dev/null || true
        rm -rf gui/build
        print_success "Build directories cleaned"
    fi
    
    check_dependencies
    build_cli
    build_gui
    
    if [ "$CREATE_RELEASE" = true ]; then
        create_release_package
    fi
    
    print_section "Build Complete!"
    echo -e "${GREEN}All builds successful!${NC}"
    echo ""
    
    if [ "$BUILD_CLI" = true ] && [ -f "bin/mtklogo" ]; then
        echo "CLI: ./bin/mtklogo"
    fi
    
    if [ "$BUILD_GUI" = true ] && [ -f "gui/build/bootmod-gui" ]; then
        echo "GUI: ./gui/build/bootmod-gui"
    fi
    echo ""
}

main "$@"
