#!/bin/bash

# Build and test script for Bootmod

set -e

echo "======================================"
echo "BootMod - Build and Test Script"
echo "======================================"
echo ""

# Check for dependencies
echo "Checking dependencies..."

if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install build-essential."
    exit 1
fi

if ! ldconfig -p | grep -q libz.so; then
    echo "Error: zlib not found. Please install zlib1g-dev."
    exit 1
fi

if ! ldconfig -p | grep -q libpng; then
    echo "Error: libpng not found. Please install libpng-dev."
    exit 1
fi

echo "All dependencies found!"
echo ""

# Clean previous build
echo "Cleaning previous build..."
make clean 2>/dev/null || true
echo ""

# Build with Make
echo "Building with Make..."
make
echo ""

# Check if binary was created
if [ ! -f bin/mtklogo ]; then
    echo "Error: Build failed - binary not found"
    exit 1
fi

echo "Build successful!"
echo ""

# Test the binary
echo "Testing binary..."
./bin/mtklogo help
echo ""

# Try to find a logo.bin to test with
if [ -f "../logo.bin" ]; then
    echo "Found logo.bin, running info command..."
    ./bin/mtklogo info ../logo.bin
elif [ -f "../../logo.bin" ]; then
    echo "Found logo.bin, running info command..."
    ./bin/mtklogo info ../../logo.bin
else
    echo "No logo.bin found for testing, but build is successful!"
fi

echo ""
echo "======================================"
echo "Build completed successfully!"
echo "Binary location: bin/mtklogo"
echo "======================================"
echo ""
echo "To install: sudo make install"
echo "To use: ./bin/mtklogo help"
