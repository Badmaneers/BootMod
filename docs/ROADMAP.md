# BootMod Development Roadmap

This document outlines the development plan for BootMod, with a focus on expanding chipset support and adding advanced features.

## 🎯 Vision

**BootMod aims to be the universal boot logo customization tool for all Android devices, regardless of chipset manufacturer.**

## 📅 Release Schedule

| Version | Release Date | Focus | Status |
|---------|-------------|-------|--------|
| v1.0.0 | 2025-12-05 | MTK Foundation | ✅ Complete |
| v2.0.0 | Q1 2026 | Qualcomm Support | 🚧 In Progress |
| v2.5.0 | Q2 2026 | Cross-Platform & GUI | 📋 Planned |
| v3.0.0 | Q3 2026 | Mobile & Advanced | 📋 Planned |

---

## ✅ v1.0.0 - MTK Foundation (Complete)

**Release Date:** December 5, 2025  
**Status:** ✅ Released

### Completed Features
- [x] MTK logo.bin parser
- [x] Unpack functionality with PNG output
- [x] Repack functionality with zlib compression
- [x] Multiple color format support (RGBA, BGRA, RGB565)
- [x] Smart dimension detection (720x1600, 1080x2400, 36x50, 28x28)
- [x] Info command for file inspection
- [x] Slot selection for targeted extraction
- [x] 1-based numbering system
- [x] Lossless round-trip verification
- [x] Linux CLI support
- [x] CMake and Makefile build systems
- [x] Comprehensive documentation
- [x] MIT License
- [x] GitHub repository setup

### Technical Achievements
- ✅ Proper MTK header handling (512 bytes, magic 0x88168858)
- ✅ Zlib compression level 9
- ✅ Automatic aspect ratio detection
- ✅ Support for 140+ logos in single file
- ✅ Memory-efficient processing
- ✅ Error handling and validation

---

## 🚧 v2.0.0 - Qualcomm Support (In Development)

**Planned Release:** Q1 2026 (January-March)  
**Status:** 🚧 Research & Development

### Primary Goals
1. **Qualcomm splash.img support** - Full read/write capability
2. **Unified interface** - Single tool for all chipsets
3. **Auto-detection** - Automatically identify chipset type
4. **Format conversion** - Convert between MTK and Qualcomm formats

### Qualcomm splash.img Research Tasks

#### Phase 1: Format Analysis (Weeks 1-2)
- [ ] Study splash.img file structure
- [ ] Identify header format and magic numbers
- [ ] Analyze image storage format (RLE compression)
- [ ] Document resolution support
- [ ] Test with various Snapdragon devices
  - [ ] Snapdragon 660/665
  - [ ] Snapdragon 720G/730/732G
  - [ ] Snapdragon 765G/768G/778G
  - [ ] Snapdragon 845/855/865/870/888
  - [ ] Snapdragon 8 Gen 1/2

#### Phase 2: Parser Implementation (Weeks 3-4)
- [ ] Create `qualcomm_format.h` header
- [ ] Implement `qualcomm_format.cpp`
  - [ ] SplashHeader structure
  - [ ] Image metadata parser
  - [ ] RLE decompression
  - [ ] Image extraction to PNG
- [ ] Add to main bootmod architecture
- [ ] Unit tests for parser

#### Phase 3: Repacker Implementation (Weeks 5-6)
- [ ] RLE compression algorithm
- [ ] Image encoding from PNG
- [ ] Header generation
- [ ] Metadata calculation
- [ ] File assembly
- [ ] Validation and checksums

#### Phase 4: Integration (Weeks 7-8)
- [ ] Auto-detect chipset from file
- [ ] Unified CLI commands
- [ ] Format conversion tools
- [ ] Cross-chipset validation
- [ ] Documentation updates
- [ ] Sample files and tutorials

### Qualcomm splash.img Specifications

```
Splash.img File Structure (Preliminary):

+----------------------------+
| Header                     |
|  - Magic: "SPLASH!!"       |
|  - Version                 |
|  - Image count             |
|  - Total size              |
+----------------------------+
| Image 1 Metadata           |
|  - Width                   |
|  - Height                  |
|  - Offset                  |
|  - Size                    |
|  - Compression (RLE)       |
+----------------------------+
| Image 2 Metadata           |
+----------------------------+
| ...                        |
+----------------------------+
| Image 1 Data (RLE)         |
+----------------------------+
| Image 2 Data (RLE)         |
+----------------------------+
| ...                        |
+----------------------------+
```

### Common Qualcomm Resolutions
- 720x1280 (HD)
- 1080x1920 (FHD)
- 1080x2340 (FHD+ 19.5:9)
- 1080x2400 (FHD+ 20:9)
- 1440x2560 (QHD)
- 1440x3200 (QHD+)

### New Features in v2.0
- [ ] `--chipset` flag to force chipset type
- [ ] `--convert` command to convert between formats
- [ ] `--validate` command to check file integrity
- [ ] Improved error messages for Qualcomm files
- [ ] Device database with known configurations
- [ ] Batch conversion tools

### Development Milestones
- **Milestone 1** (Week 4): splash.img parsing works
- **Milestone 2** (Week 6): splash.img repacking works
- **Milestone 3** (Week 8): Full round-trip verified
- **Release Candidate** (Week 9): Testing and bug fixes
- **v2.0.0 Release** (Week 10): Public release

---

## 📋 v2.5.0 - Cross-Platform & GUI (Planned)

**Planned Release:** Q2 2026 (April-June)  
**Status:** 📋 Planning Phase

### Primary Goals
1. **Windows native support**
2. **macOS support** (Intel + Apple Silicon)
3. **Qt6-based GUI** for all platforms
4. **Enhanced user experience**

### Platform Support

#### Windows
- [ ] MSVC build configuration
- [ ] MinGW build option
- [ ] Windows installer (NSIS/WiX)
- [ ] Portable executable
- [ ] Windows-specific optimizations
- [ ] File association (.bin, .img)

#### macOS
- [ ] Intel (x86_64) build
- [ ] Apple Silicon (ARM64) build
- [ ] Universal binary
- [ ] DMG installer
- [ ] Code signing
- [ ] Notarization
- [ ] Homebrew formula

#### Linux
- [ ] AppImage
- [ ] Flatpak
- [ ] Snap package
- [ ] .deb package (Debian/Ubuntu)
- [ ] .rpm package (Fedora/RHEL)
- [ ] AUR package (Arch)

### GUI Development (Qt6)

#### Core Features
- [ ] Main window with tabbed interface
- [ ] Drag-and-drop file loading
- [ ] Visual logo preview with zoom
- [ ] Side-by-side comparison (before/after)
- [ ] Image list with thumbnails
- [ ] Properties panel
- [ ] Status bar with progress

#### Advanced Features
- [ ] Built-in image editor
  - [ ] Crop/resize
  - [ ] Rotate/flip
  - [ ] Brightness/contrast
  - [ ] Filters
- [ ] Batch processing
- [ ] Project save/load
- [ ] Undo/redo system
- [ ] Dark/light themes
- [ ] Multi-language support

#### Device Integration
- [ ] ADB integration
- [ ] Auto-detect connected devices
- [ ] One-click flash to device
- [ ] Backup current logo before flash
- [ ] Device information display
- [ ] Flash history log

### Features List
- [ ] GUI application (Qt6)
- [ ] Windows build (MSVC + MinGW)
- [ ] macOS build (universal binary)
- [ ] Drag-and-drop interface
- [ ] Live preview
- [ ] Built-in image editor
- [ ] ADB integration
- [ ] Device auto-detection
- [ ] One-click flashing
- [ ] Logo templates library
- [ ] Multi-language support
- [ ] Auto-updates

---

## 📋 v3.0.0 - Mobile & Advanced (Planned)

**Planned Release:** Q3 2026 (July-September)  
**Status:** 📋 Concept Phase

### Android App (Separate Branch: `android`)

#### Core Features
- [ ] Kotlin-based Android app
- [ ] Material Design 3 UI
- [ ] Root and non-root modes
- [ ] Direct file modification
- [ ] Image picker integration
- [ ] Built-in image editor
- [ ] Logo backup/restore
- [ ] Support for Android 8+ (API 26+)

#### Root Mode Features
- [ ] Direct /dev/block access
- [ ] Automatic partition detection
- [ ] System-level backups
- [ ] Instant apply without reboot
- [ ] Advanced modifications

#### Non-Root Mode
- [ ] Extract from backed-up files
- [ ] Prepare flashable ZIP
- [ ] Integration with custom recovery
- [ ] Export modified files
- [ ] Share with community

#### Community Features
- [ ] Logo marketplace
- [ ] User-submitted logos
- [ ] Rating and reviews
- [ ] Download popular logos
- [ ] Upload your creations
- [ ] Follow creators
- [ ] Trending logos

### Additional Chipset Support

#### Samsung Exynos
- [ ] param.bin support
- [ ] Exynos boot logo format
- [ ] Samsung-specific tools

#### HiSilicon Kirin
- [ ] Huawei logo format
- [ ] Kirin-specific handling

#### Other Chipsets
- [ ] UNISOC (Spreadtrum)
- [ ] Nvidia Tegra
- [ ] Rockchip

### Advanced Features
- [ ] Cloud backup integration
- [ ] Google Drive sync
- [ ] Dropbox sync
- [ ] Logo versioning
- [ ] Collaborative editing
- [ ] AI-powered enhancement
  - [ ] Upscaling
  - [ ] Noise reduction
  - [ ] Style transfer
- [ ] Animation support
- [ ] Video to boot animation converter
- [ ] GIF to boot animation

---

## 💡 v4.0.0 - Enterprise (Future)

**Planned Release:** Future  
**Status:** 💡 Brainstorming

### Enterprise Features
- [ ] Batch processing for OEMs
- [ ] API for automation
- [ ] Remote device management
- [ ] Fleet management console
- [ ] Logo signing and verification
- [ ] Compliance checking
- [ ] Audit logs
- [ ] Role-based access control
- [ ] Enterprise licensing
- [ ] Priority support

### Integration Features
- [ ] OTA update integration
- [ ] MDM integration
- [ ] CI/CD pipeline support
- [ ] Docker containers
- [ ] Cloud deployment
- [ ] REST API
- [ ] Webhook support

---

## 🔬 Research & Development

### Ongoing Research
- Qualcomm splash.img RLE compression variants
- Samsung Exynos boot formats
- Encrypted boot image support
- Signed boot image handling
- UEFI boot logo support
- Chrome OS boot splash
- ARM SystemReady standards

### Proof of Concepts
- Web-based editor (WASM)
- Live device preview via ADB
- Machine learning for logo optimization
- Blockchain-based logo verification
- VR/AR logo preview

---

## 🤝 Community Involvement

### How to Contribute
1. **Testing** - Test on your device and report results
2. **Documentation** - Improve docs, add tutorials
3. **Code** - Submit PRs for features or fixes
4. **Chipset Support** - Help reverse engineer new formats
5. **Translations** - Localize to your language
6. **Device Database** - Submit device configurations

### Bounty Program (Future)
- $100 - Qualcomm splash.img implementation
- $50 - Windows GUI completion
- $50 - macOS support
- $25 - Per additional chipset support
- $10 - Significant bug fixes

---

## 📊 Success Metrics

### v2.0 Goals
- Support 50+ Qualcomm devices
- <1% error rate in round-trip tests
- 95% test coverage
- <100ms processing time per image

### v2.5 Goals
- 1000+ GUI downloads
- 4+ star rating
- 5+ contributors
- 90% Windows/macOS compatibility

### v3.0 Goals
- 10,000+ app downloads
- 100+ community-submitted logos
- Featured on XDA Portal
- 10+ supported chipsets

---

## 🗓️ Next Steps (Immediate)

### This Week
1. Research Qualcomm splash.img format
2. Collect sample files from different devices
3. Begin parser prototype
4. Set up testing infrastructure

### This Month
1. Complete splash.img parser
2. Implement RLE decompression
3. Create test suite
4. Document findings

### This Quarter
1. Complete v2.0 features
2. Beta testing program
3. Release v2.0.0
4. Begin GUI development

---

**Last Updated:** December 5, 2025  
**Next Review:** January 1, 2026

*This roadmap is subject to change based on community feedback and development progress.*
