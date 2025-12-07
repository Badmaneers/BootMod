# TODO List

## ✅ Completed - GUI Enhancements (v2.1.0)

### Device Preview & Layer System
- [x] Implement linked layer system (non-destructive merging)
- [x] Replace all emoji icons with professional SVG icons (33 total)
- [x] Convert Device Preview from Dialog to separate Window
- [x] Add cross-window drag-and-drop support
- [x] Disable Device Preview for Snapdragon format
- [x] Fix Snapdragon project loading with enhanced file search
- [x] Fix Device Preview window layout and icon containment
- [x] Add C++17 filesystem support to build system

## ✅ Completed - Qualcomm Support (v2.0.0)

### Qualcomm splash.img Implementation
- [x] Research splash.img file format
- [x] Collect sample files from various Snapdragon devices
- [x] Document header structure and magic numbers
- [x] Implement gzip decompression algorithm
- [x] Implement gzip compression algorithm
- [x] Create splash.h header file
- [x] Create splash.cpp implementation
- [x] Add splash.img detection to main code
- [x] Integrate with unpack command
- [x] Integrate with replace command
- [x] Test with Snapdragon devices (OPPO/OnePlus/Realme)
- [x] Verify round-trip lossless extraction
- [x] Add Qualcomm-specific CLI commands (extract, replace)
- [x] GUI integration for splash.img files
- [x] Format auto-detection (MTK vs Snapdragon)
- [x] Project mode support for both formats

## 🔥 Critical Priority - Cross-Platform (v2.5.0)

### Core Functionality (MTK)
- [ ] Add error recovery for corrupted logo.bin files
- [ ] Implement verbose mode (-v, --verbose)
- [ ] Add dry-run mode for repack (--dry-run)
- [ ] Support for logo.bin files with non-standard headers
- [ ] Add checksum verification

### Documentation
- [ ] Add man page (mtklogo.1)
- [ ] Create video tutorial
- [ ] Add troubleshooting guide with common devices
- [ ] Document all supported devices

### Testing
- [ ] Unit tests for core functions
- [ ] Integration tests
- [ ] Fuzzing tests for robustness
- [ ] Test on various MTK devices
- [ ] Create test suite with sample logo.bin files

## Medium Priority

### Features
- [ ] Batch processing multiple logo.bin files
- [ ] Config file support (~/.mtklogorc)
- [ ] Progress bar for large files
- [ ] Multi-threaded compression/decompression
- [ ] Backup and restore functionality
- [ ] Logo comparison tool
- [ ] Auto-detect optimal color mode
- [ ] Support for animated boot logos (video)

### Cross-Platform
- [ ] Windows build with MinGW/MSVC (moved to Critical)
- [ ] macOS build and testing (moved to Critical)
- [ ] Static binary releases (moved to Critical)
- [ ] AppImage for Linux (moved to Critical)
- [ ] Portable Windows exe (moved to Critical)
- [ ] Homebrew formula for macOS (moved to Critical)
- [ ] Chocolatey package for Windows (moved to Critical)

### Code Quality
- [ ] Code coverage reports
- [ ] Static analysis (cppcheck, clang-tidy)
- [ ] Memory leak detection (valgrind)
- [ ] Performance profiling
- [ ] Optimize compression speed
- [ ] Reduce memory usage for large files

## Low Priority (Future)

### GUI (Main Branch)
- [x] Qt-based GUI framework
- [x] Drag-and-drop support
- [x] Visual logo preview
- [x] Linked layer system (non-destructive merge)
- [x] Device Preview window (experimental, has bugs)
- [x] Professional SVG icon set
- [ ] Built-in image editor
- [ ] ADB integration
- [ ] Device auto-detection
- [ ] One-click flash
- [ ] Logo templates library
- [ ] Animation preview improvements
- [ ] Device Preview bug fixes and enhancements
- [ ] Community logo marketplace

### Android App (Separate Branch)
- [ ] Android project setup
- [ ] Root detection and handling
- [ ] Non-root mode (extract only)
- [ ] Material Design UI
- [ ] Direct logo.bin modification
- [ ] Built-in image editor for Android
- [ ] Logo backup/restore
- [ ] Share logos with community
- [ ] Auto-backup before modification
- [ ] Undo/redo support
- [ ] Live boot animation preview

### Advanced Features
- [ ] Logo versioning system
- [ ] Diff tool for logo.bin files
- [ ] Merge multiple logo.bin files
- [ ] Convert between MTK formats
- [ ] Support for other bootloader formats (fastboot, etc.)
- [ ] Cloud backup integration
- [ ] Logo analytics (most popular, trending)
- [ ] AI-powered logo suggestions
- [ ] Logo quality enhancement

## Documentation
- [ ] API documentation (Doxygen)
- [ ] Architecture diagrams
- [ ] Format specification document
- [ ] Developer guide
- [ ] Wiki pages
- [ ] FAQ section
- [ ] Device compatibility database

## Community
- [ ] Set up Discord server
- [ ] Create XDA thread
- [ ] Reddit community
- [ ] Logo showcase website
- [ ] Monthly logo competition
- [ ] Tutorial videos
- [ ] Live streams

## Infrastructure
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Automated testing on push
- [ ] Automated releases
- [ ] Code coverage reporting
- [ ] Performance benchmarks
- [ ] Website for documentation
- [ ] Download statistics

## Completed ✅
- [x] Core unpack functionality
- [x] Core repack functionality
- [x] Info command
- [x] Multiple color mode support
- [x] Dimension detection
- [x] Zlib compression
- [x] CMake build system
- [x] Makefile
- [x] README.md
- [x] LICENSE
- [x] .gitignore
- [x] 1-based numbering
- [x] Smart aspect ratio detection
- [x] Round-trip testing
- [x] Basic error handling
- [x] Qt6 GUI with project workflow (v1.5.0)
- [x] Qualcomm Snapdragon splash.img support (v2.0.0)
- [x] Format auto-detection (v2.0.0)
- [x] lodepng integration (v2.0.0)
- [x] Gzip compression/decompression (v2.0.0)
- [x] BMP format support (8/24/32-bit) (v2.0.0)
- [x] Alpha channel handling (v2.0.0)
- [x] Format preservation (v2.0.0)
- [x] Dual format GUI support (v2.0.0)
- [x] Project mode for both formats (v2.0.0)
- [x] Build script improvements (v2.0.0)

---

*Last updated: 2025-12-06*
