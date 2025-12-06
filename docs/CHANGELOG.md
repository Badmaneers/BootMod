# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned for v2.5.0
- Windows native build
- macOS support (Intel + Apple Silicon)
- Qt6-based GUI enhancements
  - Built-in image editor with cropping/resizing
  - Theme customization (Light/Dark/Custom)
- ADB integration for direct device flashing
- One-click device backup/restore
- Portable builds (AppImage, DMG, installer)

### Planned for v3.0.0
- Android mobile app
- Samsung Exynos support
- HiSilicon Kirin support
- Logo marketplace
- Cloud backup integration

## [2.0.0] - 2025-12-06

### Added - Qualcomm Snapdragon Support
- **Full Snapdragon splash.img support** for OPPO/OnePlus/Realme devices
  - Unpack splash.img to extract all splash screens as PNG
  - Replace individual images and repack
  - Format auto-detection (MTK vs Snapdragon)
- **lodepng library integration** (included in external/ directory)
  - PNG encoding/decoding
  - Gzip compression/decompression
  - Raw deflate stream handling
- **splash.h/splash.cpp** implementation
  - DDPH header support (optional)
  - OPPO_SPLASH header parsing (offset 0x4000)
  - Metadata array handling (128 max entries)
  - Gzip-compressed BMP data (offset 0x8000)
- **Multi-format BMP support**
  - 8-bit indexed color with palette
  - 24-bit RGB (BGR byte order)
  - 32-bit BGRA with alpha channel
  - Bottom-up scanline order handling
- **Format preservation**
  - Detects original BMP bit depth (24 or 32-bit)
  - Maintains format during repack operations
  - Preserves file structure and metadata
- **Alpha channel handling**
  - Forces opaque pixels (alpha=255) for splash screens
  - Fixes transparency display issues
  - Matches device behavior

### Enhanced - GUI Support
- **Dual format support** in Qt6 GUI
  - Works seamlessly with both logo.bin and splash.img
  - Format auto-detection on file load
  - Format-specific file naming (logo_N vs image_N)
- **Project mode enhancements**
  - Stores format type in .bootmod metadata ("mtk" or "snapdragon")
  - Reloads original splash.img when opening Snapdragon projects
  - Preserves m_splashImage pointer through workflow
  - Format-aware export (creates .bin or .img based on original)
- **File dialog updates**
  - Accept both .bin and .img extensions
  - Filter: "Logo/Splash files (*.bin *.img)"
  - Format detected after file selection
- **UI improvements**
  - Updated window title: "BootMod - Boot Logo/Splash Editor"
  - Updated welcome text to mention both formats
  - Export button text: "Export to File" (was "Export logo.bin")
  - About dialog mentions dual format support
- **Layout fixes**
  - Fixed AboutDialog overflow (height 600→680px)
  - Reduced spacing throughout dialog
  - All content visible without clipping

### Enhanced - CLI Support
- **Unified command interface**
  - Same commands work for both MTK and Snapdragon
  - `info` command shows format-specific details
  - `unpack` auto-detects and extracts accordingly
- **New Snapdragon commands**
  - `extract <splash.img> <index> <output.png>` - Single image extraction
  - `replace <splash.img> <index> <input.png> <output.img>` - Single image replacement
- **Format detection**
  - Checks MTK magic (0x88168858 at offset 0x0)
  - Checks OPPO_SPLASH magic ("SPLASH LOGO!" at offset 0x4000)
  - Returns FormatType enum (UNKNOWN, MTK_LOGO, OPPO_SPLASH)

### Fixed
- **Build system**
  - build.sh now works from any directory (auto-detects script location)
  - Version management works correctly (default 2.0.0)
  - Dependency checking improved
- **GUI export/replace**
  - m_splashImage preservation in project workflow
  - Format metadata stored and retrieved correctly
  - Replace functionality works for both formats
  - Export creates correct file format
- **Memory management**
  - Proper cleanup of lodepng allocations
  - No memory leaks in image processing
  - Efficient buffer handling for large files

### Technical Details
- **Libraries**
  - lodepng 20230410 (included in external/)
  - zlib (system library, for MTK)
  - libpng (system library, optional for MTK PNG output)
  - Qt6 6.0+ (for GUI)
- **C++ Standards**
  - C++17 required
  - std::filesystem for path operations
  - std::unique_ptr for memory management
- **Gzip Format**
  - 10-byte header (0x1f 0x8b 0x08...)
  - Raw deflate stream (lodepng_inflate/deflate)
  - 8-byte footer (CRC32 + original size)
- **BMP Format**
  - 14-byte file header
  - 40-byte info header
  - Optional palette (8-bit indexed)
  - Pixel data with row padding (multiple of 4 bytes)
  - Bottom-up storage (last row first)

### Testing
- ✅ Round-trip verification (unpack → modify → repack → verify)
- ✅ Format detection for both MTK and Snapdragon
- ✅ All 25 images extracted correctly from test splash.img
- ✅ Image replacement preserves other images (bit-perfect)
- ✅ Format preservation (32-bit BMP maintained through workflow)
- ✅ GUI project mode works for both formats
- ✅ CLI commands work for both formats
- ✅ Build script works from any directory

### Documentation
- Updated README.md with Snapdragon support
- Updated version badges (1.5.0 → 2.0.0)
- Added Snapdragon examples and usage
- Documented splash.img file format
- Added BMP format specifications
- Updated supported chipsets table

## [1.5.0] - 2025-12-05 (Previous Release)

### Added
- Initial release of BootMod (formerly MTK Logo Tool)
- Project rebranded to BootMod for multi-chipset vision
- Core unpack functionality for extracting logos from logo.bin
- Core repack functionality for rebuilding logo.bin
- Info command to inspect logo files
- Multiple color mode support:
  - RGBA Big/Little Endian
  - BGRA Big/Little Endian
  - RGB565 Big/Little Endian
- Intelligent dimension detection with aspect ratio preferences
- Zlib compression/decompression support
- Slot selection for targeted logo extraction (1-based numbering)
- Raw extraction mode (skip PNG conversion)
- Complete CMake build system
- Simple Makefile for quick builds
- Comprehensive documentation
- Build script with dependency checking
- `.gitignore` for clean repository

### Features
- Lossless round-trip extraction and repacking
- Automatic detection of battery icons (36x50), boot logos (720x1600), and other common dimensions
- Proper MTK header handling (512 bytes)
- PNG output with libpng
- Cross-platform C++11 codebase
- Command-line interface with intuitive commands

### Tested
- ✅ Full extraction of 140 logos
- ✅ Successful repacking
- ✅ Round-trip verification (extract → repack → extract)
- ✅ Byte-identical PNG outputs in round-trip
- ✅ Proper MTK header preservation
- ✅ Correct dimension detection for various image sizes

### Performance
- Fast extraction: ~0.5s for 140 logos
- Fast repacking: ~1.0s for 140 logos
- Optimal zlib compression (level 9)
- Minimal memory footprint

## [0.9.0] - 2025-12-04

### Development
- Initial C++ port from Rust mtklogo
- Core data structures implemented
- Basic parsing working
- Dimension detection algorithm developed

## [0.1.0] - 2025-12-03

### Project Start
- Project conception
- Requirements gathering
- Architecture design
