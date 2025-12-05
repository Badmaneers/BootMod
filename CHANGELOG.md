# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned for v2.0
- Qualcomm Snapdragon splash.img support
- RLE compression/decompression
- Auto-detect chipset type
- Format conversion between MTK and Qualcomm
- Device database
- Enhanced CLI with unified interface

### Planned for v2.5
- Windows native build
- macOS support (Intel + Apple Silicon)
- Qt6-based GUI
- Drag-and-drop interface
- Built-in image editor
- ADB integration
- One-click device flashing

### Planned for v3.0
- Android mobile app
- Samsung Exynos support
- HiSilicon Kirin support
- Logo marketplace
- Cloud backup integration

## [1.0.0] - 2025-12-05

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
