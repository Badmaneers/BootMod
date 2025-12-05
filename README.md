# BootMod - Universal Boot Logo Editor

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B11-orange.svg)]()

**BootMod** is a powerful cross-platform tool for customizing boot logos and splash screens on Android devices. Support for multiple chipset manufacturers including MediaTek (MTK) and Qualcomm Snapdragon.

![BootMod](https://img.shields.io/badge/Boot-Mod-green.svg)

## 🎯 Features

### Current (v1.0.0)
- ✅ **MTK logo.bin** - Full support for MediaTek devices
  - Unpack logo.bin to extract boot logos and charging animations
  - Repack modified PNG images back to logo.bin
  - Smart dimension detection (720x1600, 1080x2400, 36x50, 28x28, etc.)
  - Multiple color formats (RGBA, BGRA, RGB565 Big/Little Endian)
  - Lossless round-trip (extract → repack → extract)
- ✅ **Automatic compression** - Optimal zlib compression
- ✅ **Info command** - Inspect boot image files without extraction
- ✅ **Batch processing** - Extract specific logos with slot selection
- ✅ **Cross-platform CLI** - Linux support (Windows/macOS coming soon)

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

#### Phase 2: Cross-Platform & GUI (v2.5.0)
- [ ] Windows native build
- [ ] macOS support
- [ ] Qt-based GUI for all platforms
- [ ] Drag-and-drop interface
- [ ] Live preview
- [ ] Built-in image editor

#### Phase 3: Mobile & Advanced (v3.0.0)
- [ ] Android app (separate branch)
- [ ] Root and non-root modes
- [ ] Direct device flashing
- [ ] Cloud backup integration
- [ ] Logo marketplace

## 🏗️ Supported Chipsets

| Chipset | Status | Format | Version |
|---------|--------|--------|---------|
| **MediaTek (MTK)** | ✅ Fully Supported | logo.bin | v1.0+ |
| **Qualcomm Snapdragon** | 🚧 In Development | splash.img | v2.0+ |
| Samsung Exynos | 📋 Planned | - | v3.0+ |

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
sudo apt-get install build-essential cmake zlib1g-dev libpng-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake zlib-devel libpng-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake zlib libpng
```

### Build and Install

```bash
# Clone the repository
git clone https://github.com/Badmaneers/BootMod.git
cd bootmod

# Build
make

# Install (optional)
sudo make install
```

The executable will be in `bin/bootmod`.

## 📖 Usage

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

### Qualcomm Snapdragon Devices (Coming Soon)

```bash
# Show information about splash.img
bootmod info splash.img

# Unpack splash screen
bootmod unpack splash.img output_dir/

# Repack with custom logo
bootmod repack new_splash.img custom_logo.png
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

**Qualcomm Options (Coming Soon):**
- `--format <format>` - Output format (png, bmp, raw)
- `--resolution <WxH>` - Target resolution

#### `repack` - Rebuild Boot Image
```bash
bootmod repack <output_file> <input_files> [options]
```

**Options:**
- `--chipset <type>` - Force chipset type (mtk, qualcomm)
- `--strip-alpha` - Remove alpha channel
- `--compress <level>` - Compression level (0-9)

## 💡 Examples

### Example 1: Replace MTK Boot Logo

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

### Example 2: Qualcomm Splash Screen (Coming v2.0)

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

### Qualcomm splash.img Format (Coming Soon)

```
+------------------------+
| Header                 |
|  - Magic: "SPLASH!!"   |
|  - Image count         |
+------------------------+
| Image 1 metadata       |
| Image 2 metadata       |
+------------------------+
| Image 1 data (RLE)     |
| Image 2 data (RLE)     |
+------------------------+
```

## 🔧 Building from Source

### Using Make (Recommended)

```bash
make                 # Build
make clean          # Clean build files
make install        # Install to /usr/local/bin
make test           # Run tests
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

## 🗺️ Roadmap

### Current: v1.0.0 - MTK Foundation ✅
- [x] MTK logo.bin support
- [x] Unpack/repack functionality
- [x] Multiple color formats
- [x] Smart dimension detection
- [x] Info command
- [x] Linux CLI support

### Next: v2.0.0 - Qualcomm Support (Q1 2026) 🚧
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

### v3.0.0 - Mobile & Advanced (Q3 2026)
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
- [ ] Animation preview

### v4.0.0 - Enterprise (Future)
- [ ] Batch processing for manufacturers
- [ ] OTA update integration
- [ ] Remote device management
- [ ] Logo signing and verification
- [ ] Multi-device sync

## 🏗️ Project Structure

```
bootmod/
├── bootmod.h           # Core header file with class declarations
├── bootmod.cpp         # MTK logo.bin parsing and core functionality
├── bootmod_png.cpp     # PNG I/O and image processing
├── main.cpp            # CLI interface and command handling
├── version.h           # Version information and metadata
├── CMakeLists.txt      # CMake build configuration
├── Makefile            # Simple Makefile for quick builds
├── build.sh            # Build script with dependency checking
├── README.md           # This file - project documentation
├── ROADMAP.md          # Detailed development roadmap
├── CHANGELOG.md        # Version history and changes
├── CONTRIBUTING.md     # Contribution guidelines
├── TODO.md             # Task tracking and future features
├── LICENSE             # MIT License
├── .gitignore          # Git ignore patterns
└── .github/
    └── workflows/
        └── build.yml   # CI/CD pipeline configuration
```

### Planned Structure (v2.0+)
```
bootmod/
├── src/
│   ├── core/           # Core boot image handling
│   ├── mtk/            # MTK-specific implementation
│   ├── qualcomm/       # Qualcomm splash.img support
│   └── gui/            # GUI components (Qt6)
├── include/            # Public headers
├── tests/              # Unit and integration tests
└── docs/               # Additional documentation
```

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
2. Windows/macOS support
3. GUI development (Qt6)
4. Device testing and compatibility
5. Documentation and tutorials

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
- **Discussions**: [GitHub Discussions](- **Issues**: [GitHub Issues](https://github.com/Badmaneers/BootMod/discussions)
- **XDA Thread**: Coming soon
- **Discord**: Coming soon

## 🌟 Star History

If you find this project useful, please star it! ⭐

---

**Made with ❤️ for Android customization**

*Universal boot logo customization for all Android devices*
