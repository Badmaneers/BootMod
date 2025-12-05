# TODO List

## 🔥 Critical Priority - Qualcomm Support (v2.0)

### Qualcomm splash.img Implementation
- [ ] Research splash.img file format
- [ ] Collect sample files from various Snapdragon devices
- [ ] Document header structure and magic numbers
- [ ] Implement RLE decompression algorithm
- [ ] Implement RLE compression algorithm
- [ ] Create qualcomm_format.h header file
- [ ] Create qualcomm_format.cpp implementation
- [ ] Add splash.img detection to main code
- [ ] Integrate with unpack command
- [ ] Integrate with repack command
- [ ] Test with Snapdragon 660/720G/855/888 devices
- [ ] Verify round-trip lossless extraction
- [ ] Add Qualcomm-specific tests

## High Priority

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
- [ ] Windows build with MinGW/MSVC
- [ ] macOS build and testing
- [ ] Static binary releases
- [ ] AppImage for Linux
- [ ] Portable Windows exe
- [ ] Homebrew formula for macOS
- [ ] Chocolatey package for Windows

### Code Quality
- [ ] Code coverage reports
- [ ] Static analysis (cppcheck, clang-tidy)
- [ ] Memory leak detection (valgrind)
- [ ] Performance profiling
- [ ] Optimize compression speed
- [ ] Reduce memory usage for large files

## Low Priority (Future)

### GUI (Main Branch)
- [ ] Qt-based GUI framework
- [ ] Drag-and-drop support
- [ ] Visual logo preview
- [ ] Built-in image editor
- [ ] ADB integration
- [ ] Device auto-detection
- [ ] One-click flash
- [ ] Logo templates library
- [ ] Animation preview
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

---

*Last updated: 2025-12-05*
