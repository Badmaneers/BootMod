# BootMod - Universal Boot Logo Editor

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)]()
[![Version](https://img.shields.io/badge/version-2.0.0-brightgreen.svg)]()

**BootMod** is a powerful cross-platform tool for customizing boot logos and splash screens on Android devices. Features both a modern Qt6 GUI and command-line interface supporting **MediaTek** and **Qualcomm Snapdragon** chipsets.

### Current: v2.0.0 - Dual Chipset Support ✅
- ✅ Qualcomm Snapdragon splash.img support (OPPO/OnePlus/Realme)
- ✅ Splash.img unpack/repack in both CLI and GUI
- ✅ Gzip compression/decompression for Snapdragon
- ✅ Auto-detect chipset type (MTK vs Snapdragon)
- ✅ Unified command interface for both formats
- ✅ Format preservation (24-bit vs 32-bit BMP)
- ✅ Alpha channel handling
- ✅ GUI support for both chipsets

### Next: v2.5.0 - Cross-Platform & Enhanced GUI (Q2 2025) 🚧
- [ ] Windows native build (MSVC/MinGW)
- [ ] macOS support (Intel + Apple Silicon)
- [ ] **Enhanced GUI Features**
  - Built-in image editor with cropping/resizing
  - Undo/Redo support
- [ ] ADB integration for direct device flashing
- [ ] One-click backup/restore
- [ ] Portable builds (AppImage, DMG, installer)

### v2.5.0 - Cross-Platform & Enhanced GUI (Q3 2025)
- [ ] Windows native build (MSVC/MinGW)
- [ ] macOS support (Intel + Apple Silicon)
- [ ] **Enhanced GUI Features**
  - Built-in image editor with cropping/resizing
  - Animation preview for charging sequences
  - Logo templates library
  - Side-by-side comparison mode
  - Batch replace multiple logos
  - Theme customization (Light/Dark/Custom)
  - Undo/Redo support
- [ ] ADB integration for direct device flashing
- [ ] One-click backup/restore
- [ ] Portable builds (AppImage, DMG, installer)d CLI tool for MediaTek (MTK) devices with planned support for Qualcomm Snapdragon.

![BootMod Logo](gui/res/bootmod_logo.svg)

## 🎯 Features

### Current (v2.0.0)

#### 🎨 Modern Qt6 GUI
- ✅ **Professional Interface** - Zilium-inspired dark theme UI
- ✅ **Project-Based Workflow** - Unpack, edit, and repack in one place
  - Create projects from logo.bin files
  - Edit logos directly in project folder
  - Live thumbnail updates after replacement
  - Export modified project to logo.bin
- ✅ **Native File Dialogs** - System-native file pickers for better UX
- ✅ **Drag & Drop Support** - Easy file loading
- ✅ **Real-time Preview** - See logo changes instantly
- ✅ **Individual Logo Operations**
  - Extract any logo as PNG
  - Replace logos with validation
  - View resolution and format info
- ✅ **Status Feedback** - Clear success/error messages
- ✅ **Project Mode Indicator** - Know when logos can be edited

#### 💻 Command-Line Interface
- ✅ **MTK logo.bin** - Full support for MediaTek devices
  - Unpack logo.bin to extract boot logos and charging animations
  - Repack modified PNG images back to logo.bin
  - Smart dimension detection (720x1600, 1080x2400, 36x50, 28x28, etc.)
  - Multiple color formats (RGBA, BGRA, RGB565 Big/Little Endian)
  - Lossless round-trip (extract → repack → extract)
- ✅ **Automatic compression** - Optimal zlib compression
- ✅ **Info command** - Inspect boot image files without extraction
- ✅ **Batch processing** - Extract specific logos with slot selection
- ✅ **Version Management** - Unified versioning across CLI and GUI

### Planned Features 🚀

#### Phase 1: Qualcomm Support (v2.0.0)
- [ ] **Snapdragon splash.img** - Full Qualcomm support
  - Unpack splash.img files
  - Repack with custom logos
  - Multiple resolution support
  - Logo animation support
- [ ] **Auto-detection** - Automatically detect chipset type
- [ ] **Unified CLI** - Single tool for all chipsets
- [ ] **Format conversion** - Convert between MTK and Qualcomm formats

#### Phase 2: Enhanced GUI & Cross-Platform (v2.5.0)
- [ ] Windows native build
- [ ] macOS support (Intel + Apple Silicon)
- [ ] **Enhanced GUI Features**
  - Built-in image editor with cropping/resizing
  - Animation preview for charging sequences
  - Logo templates library
  - Side-by-side comparison mode
  - Batch replace multiple logos
  - Theme customization (Light/Dark/Custom)
- [ ] ADB integration for direct device flashing
- [ ] One-click backup/restore

#### Phase 2: Mobile & Advanced (v3.0.0)
- [ ] Android app (separate branch)
- [ ] Root and non-root modes
- [ ] Direct device flashing
- [ ] Cloud backup integration
- [ ] Logo marketplace

## 🏗️ Supported Chipsets

| Chipset | CLI Status | GUI Status | Format | Version |
|---------|-----------|------------|--------|---------|
| **MediaTek (MTK)** | ✅ Fully Supported | ✅ Fully Supported | logo.bin | v1.0+ |
| **Qualcomm Snapdragon** | ✅ Fully Supported | ✅ Fully Supported | splash.img (OPPO/OnePlus) | v2.0+ |
| Samsung Exynos | 📋 Planned | 📋 Planned | - | v3.0+ |

## 📋 Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Examples](#examples)
- [Supported Devices](#supported-devices)
- [Building from Source](#building-from-source)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

## 🚀 Installation

### Prerequisites

**Ubuntu/Debian:**
```bash
# For CLI only
sudo apt-get install build-essential cmake zlib1g-dev libpng-dev

# For GUI (additional)
sudo apt-get install qt6-base-dev qt6-declarative-dev

# Note: lodepng library is included in the repository (external/lodepng/)
```

**Fedora/RHEL:**
```bash
# For CLI only
sudo dnf install gcc-c++ cmake zlib-devel libpng-devel

# For GUI (additional)
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel
```

**Arch Linux:**
```bash
# For CLI only
sudo pacman -S base-devel cmake zlib libpng

# For GUI (additional)
sudo pacman -S qt6-base qt6-declarative
```

### Build and Install

#### Quick Build (Recommended)
```bash
# Clone the repository
git clone https://github.com/Badmaneers/BootMod.git
cd bootmod

# Build both CLI and GUI (default version 2.0.0)
./build.sh

# Or build with specific version
./build.sh --version=2.1.0

# Build only CLI
./build.sh --cli-only

# Build only GUI
./build.sh --gui-only

# Clean build files
./build.sh --clean
```

#### Manual Build

**CLI Tool:**
```bash
make                 # Build CLI
make install        # Install to /usr/local/bin
```

**GUI Application:**
```bash
cd gui
mkdir build && cd build
cmake ..
make
```

### Binaries Location
- **CLI**: `bin/bootmod`
- **GUI**: `gui/build/bootmod-gui`

## 📖 Usage

### GUI Application

Launch the GUI application:
```bash
./gui/build/bootmod-gui
```

#### Project-Based Workflow (Recommended)

1. **Open a file** - Click "Browse..." or drag & drop logo.bin or splash.img
2. **Create Project** - Click "Unpack to Project" and select an empty folder
   - Format is automatically detected (MTK or Snapdragon)
   - All images extracted as PNG files
3. **Edit Images** - Navigate to `project_folder/images/` and edit PNG files with your favorite editor
   - MTK: `logo_N_WxH.png` format
   - Snapdragon: `image_N.png` format
   - IMPORTANT: Keep same dimensions
4. **Replace in GUI** - Click "Replace" on any image card, select your edited PNG
   - The GUI validates dimensions automatically
   - Thumbnails update in real-time
5. **Export** - Click "Export to File" to create the flashable file
   - MTK projects export to logo.bin
   - Snapdragon projects export to splash.img
   - Format is preserved from original file

#### File Mode (Quick Export)
- Open logo.bin or splash.img directly to view and export individual images
- Export specific images as PNG for inspection
- View image metadata (resolution, format, compression size)

#### GUI Features
- **Dual Format Support**: Seamlessly works with both MTK logo.bin and Snapdragon splash.img
- **Format Auto-Detection**: Automatically detects chipset type when loading files
- **Project Mode Indicator**: Shows whether you can edit (Project) or only view (File Mode)
- **Image Stats**: See resolution, format, and compressed size for each image
- **Status Messages**: Real-time feedback for all operations
- **Native Dialogs**: Uses system file pickers for familiar UX
- **File Filters**: Accept both .bin and .img extensions
- **About Dialog**: View version, features, and developer info (click "About" in status bar)

### Command-Line Interface

### MediaTek (MTK) Devices

```bash
# Show information about logo.bin
bootmod info logo.bin

# Unpack all logos
bootmod unpack logo.bin output_dir/ --mode bgrabe

# Unpack specific logos (1-based numbering)
bootmod unpack logo.bin output_dir/ --mode bgrabe --slots 1,2,69

# Repack logos
bootmod repack new_logo.bin output_dir/logo_*.png
```

### Qualcomm Snapdragon Devices

```bash
# Show information about splash.img (OPPO/OnePlus/Realme)
bootmod info splash.img

# Unpack all splash screens
bootmod unpack splash.img output_dir/

# Extract specific image as PNG
bootmod extract splash.img 0 output.png

# Replace single image
bootmod replace splash.img 0 custom_logo.png new_splash.img
```

### Command Reference

#### `info` - Display Boot Image Information
```bash
bootmod info <file>
```
Shows details about the boot image file:
- Chipset type (MTK, Qualcomm, etc.)
- Number of logos/images
- Individual image sizes
- Compression status
- Resolution information

#### `unpack` - Extract Boot Logos
```bash
bootmod unpack <file> <output_dir> [options]
```

**MTK Options:**
- `--mode <mode>` - Color mode (bgrabe, bgrale, rgbabe, rgbale, rgb565be, rgb565le)
- `--slots <1,2,3>` - Extract only specific logos
- `--raw` - Extract as raw compressed files
- `--flip` - Flip orientation

**Qualcomm Options:**
- Automatically extracts all images as PNG
- Preserves original BMP format (24-bit or 32-bit)
- Handles 8-bit indexed, 24-bit RGB, and 32-bit BGRA formats

#### `repack` - Rebuild Boot Image
```bash
bootmod repack <output_file> <input_files> [options]
```

**Options:**
- `--chipset <type>` - Force chipset type (mtk, qualcomm)
- `--strip-alpha` - Remove alpha channel
- `--compress <level>` - Compression level (0-9)

## 💡 Examples

### Example 1: Replace MTK Boot Logo (GUI Method)

```bash
# 1. Launch GUI
./gui/build/bootmod-gui

# 2. Open logo.bin file (Browse button or drag & drop)
#    - File appears in the text field
#    - Logo grid displays all images

# 3. Create a project
#    - Click "Unpack to Project"
#    - Select empty folder (e.g., ~/bootmod_project)
#    - All logos extracted automatically

# 4. Edit logos externally
#    - Navigate to ~/bootmod_project/images/
#    - Edit logo_1_720x1280.png with GIMP/Photoshop/etc.
#    - IMPORTANT: Keep same dimensions (720x1280)

# 5. Replace in GUI
#    - Click "Replace" button on Logo #1 card
#    - Select your edited logo_1_720x1280.png
#    - Thumbnail updates automatically
#    - Dimension validation ensures compatibility

# 6. Export new logo.bin
#    - Click "Export logo.bin"
#    - Save as new_logo.bin
#    - Flash to device via fastboot
```

### Example 2: Replace MTK Boot Logo (CLI Method)

```bash
# 1. Extract the boot logo (usually logo 1)
mkdir extracted
bootmod unpack logo.bin extracted/ --mode bgrabe --slots 1

# 2. Edit the PNG (keep same dimensions!)
gimp extracted/logo_001_bgrabe.png

# 3. Repack
bootmod repack new_logo.bin extracted/logo_*.png

# 4. Flash to device
adb reboot bootloader
fastboot flash logo new_logo.bin
```

### Example 3: Inspect Boot Image

```bash
# 1. Extract splash screen
bootmod unpack splash.img extracted/

# 2. Edit your custom logo
gimp extracted/splash_logo.png

# 3. Repack
bootmod repack new_splash.img extracted/splash_logo.png

# 4. Flash to device
fastboot flash splash new_splash.img
```

### Example 3: Inspect Boot Image

```bash
bootmod info logo.bin
```

Output:
```
BootMod - Boot Image Information
================================
Chipset: MediaTek (MTK)
File: logo.bin
Type: LOGO
Number of logos: 140
Block size: 2758390 bytes
Total size: 2758902 bytes

Logo Details:
  Logo 1: 51666 bytes (zlib) -> 4608000 bytes (720x1600)
  Logo 2: 786530 bytes (zlib) -> 4608000 bytes (720x1600)
  Logo 69: 1143 bytes (zlib) -> 7200 bytes (36x50 battery)
  ...
```

### Example 4: Qualcomm Splash Screen (Coming v2.0)

```bash
# 1. Extract splash screen
bootmod unpack splash.img extracted/

# 2. Edit your custom logo
gimp extracted/splash_logo.png

# 3. Repack
bootmod repack new_splash.img extracted/splash_logo.png

# 4. Flash to device
fastboot flash splash new_splash.img
```

## 📐 File Formats

### MTK logo.bin Format

```
+------------------------+
| MTK Header (512 bytes) |
|  - Magic: 0x88168858   |
|  - Size (LE)           |
|  - Type: "LOGO"        |
+------------------------+
| Logo Count             |
| Block Size             |
| Offset Table           |
+------------------------+
| Logo 1 (zlib)          |
| Logo 2 (zlib)          |
| ...                    |
+------------------------+
```

### Qualcomm splash.img Format (OPPO/OnePlus/Realme)

```
+---------------------------+
| DDPH Header (optional)    |
|  - Magic: 0x48504444      |
|  - Offset: 0x0            |
+---------------------------+
| OPPO_SPLASH Header        |
|  - Magic: "SPLASH LOGO!"  |
|  - Offset: 0x4000         |
|  - Image count            |
|  - Display width/height   |
+---------------------------+
| Metadata Array (128 max)  |
|  - Image offset           |
|  - Compressed size        |
|  - Uncompressed size      |
|  - Image name (116 bytes) |
+---------------------------+
| Compressed Images         |
|  - Offset: 0x8000         |
|  - Format: Gzip           |
|  - Data: BMP (8/24/32-bit)|
+---------------------------+
```

**Supported BMP Formats:**
- 8-bit indexed color with palette
- 24-bit RGB (BGR byte order)
- 32-bit BGRA with alpha channel

## 🔧 Building from Source

### Using Unified Build Script (Recommended)

```bash
# Build both CLI and GUI with default version (2.0.0)
./build.sh

# Build with custom version
./build.sh --version=2.1.0

# Build only CLI tool
./build.sh --cli-only

# Build only GUI application  
./build.sh --gui-only

# Clean all build artifacts
./build.sh --clean

# Show help
./build.sh --help
```

The build script automatically:
- Checks for required dependencies (gcc, qt6, zlib, libpng)
- Updates version numbers in all files
- Compiles with parallel jobs
- Provides build summary with binary locations

### Manual Build Methods

#### CLI Tool (Make)
```bash
make                 # Build
make clean          # Clean build files
make install        # Install to /usr/local/bin
make test           # Run tests
```

#### GUI Application (CMake)
```bash
cd gui
mkdir build && cd build
export BOOTMOD_VERSION="2.0.0"  # Optional: Set version
cmake ..
make -j$(nproc)
```

### Dependencies

**Required for CLI:**
- GCC/Clang with C++17 support
- CMake 3.16+
- zlib development files
- libpng development files

**Additional for GUI:**
- Qt6 (6.0+)
  - Qt6Core
  - Qt6Gui
  - Qt6Quick
  - Qt6Qml
  - Qt6Widgets

## 🗺️ Roadmap

### Current: v2.0.0 - Dual Chipset Support ✅
- [x] Qualcomm Snapdragon splash.img support
- [x] Qt6-based GUI with modern dark theme
- [x] Project-based workflow for both chipsets
- [x] Format auto-detection and preservation
- [x] Live thumbnail updates after image replacement
- [x] Native file dialogs for both .bin and .img files
- [x] Alpha channel handling for splash screens
- [x] BMP format preservation (24-bit vs 32-bit)
- [x] Drag & drop support
- [x] Individual logo export/replace operations
- [x] Real-time status feedback
- [x] Professional branding and logo
- [x] Unified build system with version management
- [x] CLI tool with MTK logo.bin support
- [x] Linux support

### Next: v2.0.0 - Qualcomm Support (Q2 2025) 🚧
- [ ] Qualcomm splash.img parser
- [ ] Splash.img unpack/repack
- [ ] RLE compression/decompression
- [ ] Auto-detect chipset type
- [ ] Unified command interface
- [ ] Cross-format conversion tools
- [ ] Extended device database

### v2.5.0 - Cross-Platform & GUI (Q2 2026)
- [ ] Windows native build (MSVC/MinGW)
- [ ] macOS support (Intel + Apple Silicon)
- [ ] Qt6-based GUI
  - Drag-and-drop interface
  - Live logo preview
  - Side-by-side comparison
  - Built-in image editor
- [ ] ADB integration
- [ ] One-click device flashing
- [ ] Logo templates library

### v3.0.0 - Mobile & Advanced (Q4 2025)
- [ ] **Android App** (separate branch)
  - Root and non-root modes
  - Direct logo.bin/splash.img modification
  - Material Design 3 UI
  - Logo backup/restore
  - Community marketplace
- [ ] Samsung Exynos support
- [ ] HiSilicon Kirin support
- [ ] Cloud backup integration
- [ ] AI-powered logo enhancement
- [ ] Animation preview in all platforms

### v4.0.0 - Enterprise (Future)
- [ ] Batch processing for manufacturers
- [ ] OTA update integration
- [ ] Remote device management
- [ ] Logo signing and verification
- [ ] Multi-device sync

## 🏗️ Project Structure

```
bootmod/
├── src/                    # CLI source files
│   ├── bootmod.cpp         # MTK logo.bin parsing and core functionality
│   ├── bootmod_png.cpp     # PNG I/O and image processing
│   └── main.cpp            # CLI interface and command handling
├── include/                # Header files
│   └── bootmod.h           # Core header with class declarations
├── gui/                    # Qt6 GUI application
│   ├── src/                # GUI C++ source files
│   │   ├── main.cpp        # GUI entry point with version exposure
│   │   ├── logofile.cpp    # Logo file operations and project management
│   │   ├── logolistmodel.cpp # Qt model for logo list
│   │   ├── thumbnailprovider.cpp # QML image provider
│   │   └── appcontext.cpp  # Application context singleton
│   ├── qml/                # QML UI files
│   │   ├── Main.qml        # Main application window
│   │   └── AboutDialog.qml # About dialog with developer info
│   ├── res/                # Resources
│   │   └── bootmod_logo.svg # Application logo (48x48 SVG)
│   ├── build/              # CMake build directory (generated)
│   └── CMakeLists.txt      # GUI build configuration with version management
├── build/                  # CLI build artifacts (generated)
├── bin/                    # CLI compiled binary (generated)
├── build.sh                # Unified build script with version management
├── Makefile                # CLI build configuration
├── CMakeLists.txt          # GUI CMake configuration
├── README.md               # This file - comprehensive documentation
├── LICENSE                 # MIT License
└── .gitignore              # Git ignore patterns (includes Qt/build artifacts)
```

### Key Components

#### CLI Tool (`src/`)
- **bootmod.cpp**: Core MTK logo.bin parser, LogoImage class, MtkHeader, compression/decompression
- **bootmod_png.cpp**: PNG loading/saving with ImageUtils, color format conversions
- **main.cpp**: Command-line interface with info/unpack/repack commands

#### GUI Application (`gui/`)
- **logofile.cpp/h**: Backend for file operations, project management, native dialogs
- **logolistmodel.cpp/h**: Qt AbstractListModel for QML GridView
- **thumbnailprovider.cpp/h**: QQuickImageProvider for efficient thumbnail rendering
- **appcontext.cpp/h**: Singleton for sharing thumbnail provider across components
- **Main.qml**: Main UI with Zilium-inspired design, project workflow, drag-drop
- **AboutDialog.qml**: About dialog with version, features, developer links

#### Build System
- **build.sh**: Unified bash script
  - Version management (--version flag)
  - Selective builds (--cli-only, --gui-only)
  - Dependency checking
  - Parallel compilation
  - Updates Makefile and CMakeLists.txt versions
- **Makefile**: CLI builds with VERSION variable
- **gui/CMakeLists.txt**: Qt6 GUI builds with BOOTMOD_VERSION from environment

## 🎯 Supported Devices

### MediaTek (Current Support)
- Xiaomi Redmi series
- Realme devices
- OPPO devices
- Vivo devices
- Tecno/Infinix devices
- Generic MTK chipsets

### Qualcomm Snapdragon (Planned v2.0)
- Samsung Galaxy series
- OnePlus devices
- Xiaomi Mi/Poco series
- Motorola devices
- Sony Xperia
- ASUS ROG Phone
- Google Pixel (older)
- Generic Snapdragon chipsets

## 🤝 Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

### Priority Areas
1. **Qualcomm splash.img implementation** (High priority)
2. Windows/macOS support and testing
3. GUI enhancements and new features
4. Device testing and compatibility reports
5. Documentation, tutorials, and video guides
6. Bug reports and fixes
7. Logo templates and presets

### Development Setup
```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/BootMod.git
cd bootmod

# Build and test
./build.sh --version=dev

# Make changes and test
./bin/bootmod info test_files/logo.bin
./gui/build/bootmod-gui

# Submit PR with clear description
```

## ⚠️ Warnings

**IMPORTANT:** Modifying boot images may brick your device!

Before proceeding:
1. ✅ **Backup** your original files
2. ✅ **Unlock bootloader** (will wipe data)
3. ✅ **Test** on similar device first
4. ✅ **Verify** file sizes match partition
5. ✅ **Use at your own risk**

### Recovery
If your device won't boot:
- Boot to recovery (TWRP/Stock)
- Restore backup via recovery
- Use manufacturer flash tool
- Contact device support

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

## 🙏 Credits

- Inspired by [mtklogo](https://github.com/arlept/mtklogo) (Rust)
- Based on [mtkimg](https://github.com/rom1nux/mtkimg) research
- Community contributions from XDA Developers
- Qualcomm format research from various sources

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Badmaneers/BootMod/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Badmaneers/BootMod/discussions)
- **Telegram**: [@DumbDragon](https://t.me/DumbDragon)
- **XDA Thread**: Coming soon
- **Discord**: Coming soon

## 🌟 Screenshots

### GUI Application
![BootMod GUI - Main Window](screenshots/main-window.png)
*Modern Qt6 interface with project-based workflow*

![BootMod GUI - Logo Grid](screenshots/logo-grid.png)
*Visual logo browser with thumbnails and metadata*

![BootMod GUI - About Dialog](screenshots/about-dialog.png)
*About dialog with features and developer info*

> **Note**: Screenshots coming soon. The GUI features a professional dark theme inspired by Zilium with native system dialogs.

## 🌟 Star History

If you find this project useful, please star it! ⭐

---

**Made with ❤️ for Android customization**

*Universal boot logo customization for all Android devices*
