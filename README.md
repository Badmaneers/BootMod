# BootMod — Universal Boot Logo Editor

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)]()
[![Version](https://img.shields.io/badge/version-2.5.0-brightgreen.svg)]()

**BootMod** is an open-source tool for viewing, editing, and repacking Android boot logo and splash screen images. It ships with both a modern **Qt6 GUI** and a **command-line interface**, and supports all three common Android bootlogo formats:

| Format | Chipset | File |
|---|---|---|
| MTK logo.bin | MediaTek | `logo.bin` |
| Snapdragon splash.img | Qualcomm (OPPO/OnePlus/Realme) | `splash.img` |
| Samsung up_param | Samsung (Exynos/Snapdragon) | `up_param.bin` / `up_param.img` |

---

## Features

### GUI (Qt6 QML)
- Dark-themed modern UI
- Open logo files directly or via drag & drop
- Thumbnail grid — view all boot logos at a glance
- **Project workflow** — unpack a file to an editable folder, edit individual images, export back to the original format
- Per-image **Export** and **Edit/Replace** buttons
- Live thumbnail refresh after every edit
- Built-in bitmap editor integration
- Open existing projects without needing the original binary

### Command-Line Interface (CLI)
- `unpack` — extract all logos from any supported file
- `repack` — rebuild a binary from modified images
- `info` — display file metadata without extraction
- `extract` / `replace` — single-image operations (Snapdragon)
- Auto-detects file format from binary magic bytes

---

## Supported Formats

### MediaTek `logo.bin`
- Detects MTK magic `0x88168858` at offset 0
- Stores images as zlib-compressed raw pixel data (BGRA8888 / RGB565)
- Round-trip lossless: unpack → edit PNGs → repack

### Qualcomm `splash.img`
- Detects `SPLASH LOGO!` magic at offset `0x4000`
- Images stored as gzip-compressed BMP data
- Supports OPPO, OnePlus, Realme, and other Snapdragon devices
- `extract` / `replace` commands for single-image operations
- GUI project mode: view-only if original splash.img is unavailable

### Samsung `up_param`
- Detects POSIX `ustar` tar magic at offset 257
- Images are standard JPEG files packed in a tar archive
- Full read/write support: unpack, replace individual JPEGs, repack

---

## Building

### Requirements
- CMake 3.16+
- C++17-capable compiler (GCC/Clang on Linux, MSVC on Windows)
- Qt 6.x (`Core`, `Gui`, `Quick`, `Qml`, `Widgets`)
- zlib
- libpng

---

### Linux

Install dependencies on Debian/Ubuntu:
```bash
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev \
                 libqt6quick6 libz-dev libpng-dev
```

#### Build everything (GUI + CLI)
```bash
./build.sh
```

#### Build CLI only
```bash
./build.sh --cli-only
# Binary: bin/bootmod
```

#### Build GUI only
```bash
./build.sh --gui-only
# Binary: gui/build/bootmod-gui
```

#### Release build
```bash
./build.sh --release
```

#### Manual CLI build
```bash
make -j$(nproc)
# Binary: bin/bootmod
```

#### Manual GUI build
```bash
mkdir -p gui/build && cd gui/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# Binary: gui/build/bootmod-gui
```

---

### Windows

#### Requirements
- Visual Studio 2022 (with C++ Desktop workload)
- [vcpkg](https://github.com/microsoft/vcpkg) — install `zlib` and `libpng` for `x64-windows`:
  ```bat
  vcpkg install zlib:x64-windows libpng:x64-windows
  ```
- Qt 6.x for Windows — set the `Qt6_DIR` environment variable to your Qt install, e.g.:
  ```bat
  set Qt6_DIR=C:\Qt\6.x.x\msvc2022_64\lib\cmake\Qt6
  ```

#### Build everything (GUI + CLI)
Run from a **Developer Command Prompt for VS 2022**:
```bat
build-windows.bat
```
Output binaries are staged in `dist-win\`:
- `dist-win\bootmod.exe` — CLI
- `dist-win\bootmod-gui.exe` — GUI (with all Qt runtime DLLs deployed via `windeployqt`)

The script will also copy the required vcpkg runtime DLLs (`zlib1.dll`, `libpng16.dll`) from `C:\vcpkg\installed\x64-windows\bin\`. If vcpkg is installed elsewhere, edit the `VCPKG_TOOLCHAIN` and `VCPKG_TRIPLET` variables at the top of `build-windows.bat`.

---

## CLI Usage

```
bootmod <command> [options]

Commands:
  unpack <file> <output_dir>      Extract all logos/images
  repack <output> <file1> ...     Repack images into binary
  info   <file>                   Display file information
  extract <splash.img> <index> <out.png>
  replace <splash.img> <index> <in.png> <out.img>

Supported input files:
  logo.bin         MediaTek boot logo archive
  splash.img       Qualcomm/OPPO splash image
  up_param.bin     Samsung up_param tar archive

MTK unpack options:
  --mode <mode>    Color mode: bgrale (default), bgrabe, rgb565le, rgb565be
  --slots 0,1,2    Extract only specific slots
  --raw            Extract raw compressed .z files

Repack options:
  --strip-alpha    Remove alpha channel when repacking
```

### Examples

```bash
# MediaTek
bootmod unpack logo.bin extracted/
bootmod repack new_logo.bin extracted/logo_*.png

# Qualcomm
bootmod unpack splash.img extracted/
bootmod extract splash.img 0 logo0.png
bootmod replace splash.img 0 new_logo.png output.img

# Samsung
bootmod unpack up_param.bin extracted/
bootmod repack up_param_new.tar extracted/*.jpg

# Get info on any file
bootmod info logo.bin
bootmod info splash.img
bootmod info up_param.bin
```

---

## GUI Workflow

### Opening a file
1. Launch `bootmod-gui`
2. Click **Open File** or drag a `logo.bin` / `splash.img` / `up_param.bin` onto the window
3. All boot logos appear as thumbnails in the grid

### Editing logos
1. Click **Unpack to Project** to extract to an editable folder
2. Click **Edit** on any thumbnail to open the built-in image editor, or **Export** to save as a file
3. Click **Edit/Replace** to replace a logo with an image from disk
4. Thumbnails update live after every save
5. Click **Export File** to repack and save the modified binary

### Opening an existing project
1. Click **Open Project** and choose a previously unpacked project folder
2. BootMod detects the format automatically (no original binary required for viewing)

---

## Project Structure

```
bootmod/
├── bin/                    CLI binary output (Linux)
├── build/                  CLI build artifacts (Linux)
├── build-cli-win/          CLI build artifacts (Windows)
├── build-gui-win/          GUI build artifacts (Windows)
├── dist-win/               Windows distribution output
├── build-windows.bat       Windows build script
├── build.sh                Linux build script
├── docs/                   Documentation
├── external/
│   └── lodepng/            PNG codec (header-only)
├── gui/
│   ├── bitmapeditor/       Embedded bitmap editor library
│   ├── build/              GUI build artifacts
│   ├── qml/                Qt Quick UI source
│   ├── res/                Icons and SVG assets
│   └── src/                GUI C++ sources
├── include/
│   ├── bootmod.h           Format types and MTK image API
│   ├── splash.h            Snapdragon SplashImage API
│   ├── upparam.h           Samsung UpParam API
│   └── version.h           Version definitions
└── src/
    ├── bootmod.cpp         Format detection + MTK implementation
    ├── bootmod_png.cpp     PNG conversion utilities
    ├── main.cpp            CLI entry point
    ├── splash.cpp          Snapdragon implementation
    └── upparam.cpp         Samsung implementation
```

---

## Acknowledgements

### QtBitmapEditor
The built-in image editor is based on [QtBitmapEditor](https://github.com/0xMartin/QtBitmapEditor) by **Martin Krčma ([@0xMartin](https://github.com/0xMartin))**.

> Qt Bitmap Editor is a multi-platform raster image editor written in C++ and utilizing the Qt framework.

Licensed under the MIT License. Source included in `gui/bitmapeditor/`.

---

## License

MIT — see [LICENSE](LICENSE).
