# Project info
VERSION = 555
PROJECT = bootmod

# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -Iinclude -DVERSION=\"$(VERSION)\"

# Libraries
LIBS = -lz -lpng

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/bootmod.cpp $(SRC_DIR)/bootmod_png.cpp $(SRC_DIR)/main.cpp
OBJECTS = $(BUILD_DIR)/bootmod.o $(BUILD_DIR)/bootmod_png.o $(BUILD_DIR)/main.o
TARGET = $(PROJECT)

# Default target
all: $(BIN_DIR)/$(TARGET)

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Build executable
$(BIN_DIR)/$(TARGET): $(BUILD_DIR) $(BIN_DIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(TARGET) $(OBJECTS) $(LIBS)
	@echo "Built: $(BIN_DIR)/$(TARGET)"

# Compile source files
$(BUILD_DIR)/bootmod.o: $(SRC_DIR)/bootmod.cpp $(INCLUDE_DIR)/bootmod.h $(INCLUDE_DIR)/version.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/bootmod_png.o: $(SRC_DIR)/bootmod_png.cpp $(INCLUDE_DIR)/bootmod.h $(INCLUDE_DIR)/version.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/bootmod.h $(INCLUDE_DIR)/version.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o
	rm -rf $(BIN_DIR)
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build files"

# Install to system
install: $(BIN_DIR)/$(TARGET)
	install -D $(BIN_DIR)/$(TARGET) /usr/local/bin/$(TARGET)
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Uninstall from system
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled from /usr/local/bin/$(TARGET)"

# Run tests
test: $(BIN_DIR)/$(TARGET)
	@echo "Running tests..."
	@if [ -f ../logo.bin ]; then ./$(BIN_DIR)/$(TARGET) info ../logo.bin; else echo "No logo.bin file found for testing"; fi

# Help
help:
	@echo "BootMod - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the executable (default)"
	@echo "  clean      - Remove build files"
	@echo "  install    - Install to /usr/local/bin"
	@echo "  uninstall  - Remove from /usr/local/bin"
	@echo "  test       - Run basic tests"
	@echo "  help       - Show this help message"

.PHONY: all clean install uninstall test help
