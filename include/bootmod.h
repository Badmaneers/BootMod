#ifndef MTKLOGO_H
#define MTKLOGO_H

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace bootmod {

// Boot image format enumeration
enum class FormatType {
    UNKNOWN,
    MTK_LOGO,      // MediaTek logo.bin
    OPPO_SPLASH    // Snapdragon splash.img
};

// Format detection utility
FormatType detectFormat(const std::string& filename);

} // namespace bootmod

namespace mtklogo {

// Endianness enumeration
enum class Endian {
    Little,
    Big
};

// Color mode enumeration
enum class ColorMode {
    RGBA_BE,    // RGBA Big Endian
    RGBA_LE,    // RGBA Little Endian
    BGRA_BE,    // BGRA Big Endian
    BGRA_LE,    // BGRA Little Endian
    RGB565_BE,  // RGB565 Big Endian
    RGB565_LE   // RGB565 Little Endian
};

// MTK Image Type
enum class MtkType {
    RECOVERY,
    ROOTFS,
    KERNEL,
    LOGO
};

// MTK Header structure
struct MtkHeader {
    static constexpr size_t SIZE = 512;
    static constexpr uint32_t MAGIC = 0x88168858;
    static constexpr uint8_t FILL = 0xFF;

    uint32_t size;
    MtkType mtk_type;

    // Read header from buffer
    static MtkHeader read(const uint8_t* data, size_t length);
    
    // Write header to buffer
    void write(std::vector<uint8_t>& buffer) const;
    
    // Get type name as string
    std::string getTypeName() const;
};

// Logo Table structure
struct LogoTable {
    MtkHeader header;
    uint32_t logo_count;
    uint32_t block_size;
    std::vector<uint32_t> offsets;

    // Read logo table from buffer
    static LogoTable read(const uint8_t* data, size_t length);
    
    // Write logo table to buffer
    void write(std::vector<uint8_t>& buffer) const;
    
    // Get blob size for specific index
    uint32_t getBlobSize(size_t index) const;
};

// Complete Logo Image
class LogoImage {
public:
    LogoTable table;
    std::vector<std::vector<uint8_t>> blobs;

    // Read complete logo image from file
    static LogoImage readFromFile(const std::string& filename);
    
    // Read from buffer
    static LogoImage read(const uint8_t* data, size_t length);
    
    // Create from blobs
    static LogoImage createFromBlobs(const std::vector<std::vector<uint8_t>>& blobs);
    
    // Write to file
    void writeToFile(const std::string& filename) const;
    
    // Write to buffer
    std::vector<uint8_t> write() const;
    
    // Get number of logos
    size_t getLogoCount() const { return blobs.size(); }
};

// Image processing utilities
class ImageUtils {
public:
    // Decompress zlib data
    static std::vector<uint8_t> zlibDecompress(const std::vector<uint8_t>& compressed);
    
    // Compress data with zlib
    static std::vector<uint8_t> zlibCompress(const std::vector<uint8_t>& data, int level = 9);
    
    // Convert raw pixel data to PNG
    static bool saveToPNG(const std::string& filename, 
                         const std::vector<uint8_t>& pixels,
                         uint32_t width, 
                         uint32_t height,
                         ColorMode mode);
    
    // Load PNG and convert to raw pixels
    static std::vector<uint8_t> loadFromPNG(const std::string& filename,
                                           uint32_t& width,
                                           uint32_t& height,
                                           ColorMode mode);
    
    // Convert between color modes
    static std::vector<uint8_t> convertColorMode(const std::vector<uint8_t>& pixels,
                                                 ColorMode from,
                                                 ColorMode to);
    
    // Get bytes per pixel for color mode
    static uint32_t getBytesPerPixel(ColorMode mode);
    
    // Get color mode name
    static std::string getColorModeName(ColorMode mode);
    
    // Parse color mode from string
    static ColorMode parseColorMode(const std::string& name);
};

// File information structure
struct FileInfo {
    size_t id;
    bool is_compressed;
    ColorMode color_mode;

    // Generate filename for this info
    std::string getFilename() const;
    
    // Parse file info from filename
    static FileInfo fromFilename(const std::string& filename);
};

// Main operations
class MtkLogo {
public:
    // Unpack logo.bin to directory
    static bool unpack(const std::string& logo_file,
                      const std::string& output_dir,
                      ColorMode mode,
                      const std::vector<size_t>& slots = {},
                      bool extract_raw = false,
                      bool flip = false);
    
    // Repack images to logo.bin
    static bool repack(const std::vector<std::string>& input_files,
                      const std::string& output_file,
                      bool strip_alpha = false);
    
    // Guess image dimensions from size
    static std::vector<std::pair<uint32_t, uint32_t>> guessDimensions(size_t byte_size,
                                                                       ColorMode mode);
};

// Exception class
class MtkLogoException : public std::runtime_error {
public:
    explicit MtkLogoException(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace mtklogo

#endif // MTKLOGO_H
