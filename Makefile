# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Libraries
LIBS = -lz -lpng

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = bootmod.cpp bootmod_png.cpp main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = bootmod

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(TARGET) $(OBJECTS) $(LIBS)
	@echo "Built: $(BIN_DIR)/$(TARGET)"

# Compile source files
%.o: %.cpp mtklogo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS)
	rm -rf $(BIN_DIR)
	@echo "Cleaned build files"

# Install to system
install: $(TARGET)
	install -D $(BIN_DIR)/$(TARGET) /usr/local/bin/$(TARGET)
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Uninstall from system
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled from /usr/local/bin/$(TARGET)"

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	@if [ -f logo.bin ]; then \
		./$(BIN_DIR)/$(TARGET) info logo.bin; \
	else \
		echo "No logo.bin file found for testing"; \
	fi

# Help
help:
	@echo "MTK Logo Tool - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the executable (default)"
	@echo "  clean      - Remove build files"
	@echo "  install    - Install to /usr/local/bin"
	@echo "  uninstall  - Remove from /usr/local/bin"
	@echo "  test       - Run basic tests"
	@echo "  help       - Show this help message"

.PHONY: all clean install uninstall test help
