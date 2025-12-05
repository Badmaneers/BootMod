# Contributing to MTK Logo Tool

First off, thank you for considering contributing to MTK Logo Tool! It's people like you that make this tool better for everyone.

## Code of Conduct

This project and everyone participating in it is governed by respect and professionalism. Please be kind and courteous.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When you create a bug report, include as many details as possible:

- **Use a clear and descriptive title**
- **Describe the exact steps to reproduce the problem**
- **Provide specific examples** (logo.bin file, command used, etc.)
- **Describe the behavior you observed** and what you expected
- **Include screenshots** if applicable
- **System information**: OS, compiler version, library versions

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion:

- **Use a clear and descriptive title**
- **Provide a detailed description** of the suggested enhancement
- **Explain why this enhancement would be useful**
- **List any alternative solutions** you've considered

### Pull Requests

1. Fork the repo and create your branch from `main`
2. If you've added code that should be tested, add tests
3. Ensure your code follows the existing style
4. Make sure your code compiles without warnings
5. Write a clear commit message
6. Update documentation if needed

## Development Process

### Branch Structure

- `main` - Stable releases, Linux/Windows/macOS CLI and GUI
- `android` - Android app development (separate codebase)
- `feature/*` - New features
- `bugfix/*` - Bug fixes
- `experimental/*` - Experimental features

### Building

```bash
# Clone your fork
git clone https://github.com/Badmaneers/BootMod.git
cd BootMod

# Build
make

# Test
./bin/mtklogo info test_logo.bin
```

### Coding Style

- Use C++11 standard
- Indent with 4 spaces
- Use descriptive variable names
- Add comments for complex logic
- Follow RAII principles
- Avoid raw pointers when possible

Example:
```cpp
// Good
std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw MtkLogoException("Cannot open file: " + filename);
    }
    // ... rest of function
}

// Bad
unsigned char* readFile(char* filename) {
    FILE* f = fopen(filename, "rb");
    // ... manual memory management
}
```

### Testing

Before submitting a PR, test your changes:

```bash
# Test basic functionality
./bin/mtklogo info logo.bin
./bin/mtklogo unpack logo.bin test_out/ --mode bgrabe --slots 1
./bin/mtklogo repack test_logo.bin test_out/logo_*.png

# Test round-trip
mkdir original repacked
./bin/mtklogo unpack logo.bin original/ --mode bgrabe
./bin/mtklogo repack new_logo.bin original/logo_*.png
./bin/mtklogo unpack new_logo.bin repacked/ --mode bgrabe

# Compare
md5sum original/logo_001_bgrabe.png repacked/logo_001_bgrabe.png
```

## Project Roadmap

### Phase 1: Core CLI (Complete ✅)
- [x] Basic unpack/repack
- [x] Multiple color modes
- [x] Dimension detection
- [x] Info command

### Phase 2: Cross-Platform CLI (In Progress)
- [ ] Windows support
- [ ] macOS support
- [ ] Improved error handling
- [ ] Better documentation

### Phase 3: GUI Development (Planned)
- [ ] Qt-based GUI for Linux/Windows/macOS
- [ ] Drag-and-drop support
- [ ] Visual logo preview
- [ ] Integrated image editor

### Phase 4: Android App (Separate Branch)
- [ ] Native Android app
- [ ] Root and non-root support
- [ ] Material Design UI
- [ ] Community features

## Need Help?

- **Documentation**: Check the [README.md](README.md)
- **Issues**: Browse [existing issues](https://github.com/Badmaneers/BootMod/issues)
- **Discussions**: Ask in [GitHub Discussions](https://github.com/Badmaneers/BootMod/discussions)

## Recognition

Contributors will be recognized in:
- README.md contributors section
- Release notes
- Project credits

Thank you for contributing! 🎉
