#ifndef MTKLOGO_H
#define MTKLOGO_H

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace bootmod {

// Boot image format enumeration
enum class FormatType {
    UNKNOWN,
    MTK_LOGO,      // MediaTek logo.bin
    SD_SPLASH,   // Snapdragon splash.img
    SAMSUNG_UP_PARAM // Samsung up_param.tar/.img
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
                                                                       ColorMode mode) {
        std::vector<std::pair<uint32_t, uint32_t>> results;
        uint32_t bpp = ImageUtils::getBytesPerPixel(mode);

        size_t pixel_count = byte_size / bpp;

        // Common screen dimensions (portrait orientation: width < height)
        std::vector<std::pair<uint32_t, uint32_t>> common_resolutions = {
            // Modern smartphones (portrait)
            {720, 1600},   // HD+ (18:9)
            {720, 1560},   // HD+ with notch
            {720, 1520},   // HD+
            {720, 1440},   // HD+ (18:9)
            {720, 1280},   // HD (16:9)
            {1080, 2400},  // FHD+ (20:9)
            {1080, 2340},  // FHD+ with notch
            {1080, 2280},  // FHD+ (19:9)
            {1080, 2160},  // FHD+ (18:9)
            {1080, 1920},  // FHD (16:9)
            {1440, 3200},  // QHD+ (20:9)
            {1440, 3040},  // QHD+ (19:9)
            {1440, 2960},  // QHD+ (18.5:9)
            {1440, 2880},  // QHD+ (18:9)
            {1440, 2560},  // QHD (16:9)
            // Tablets / landscape screens
            {2560, 800},   // WUXGA tablet landscape
            {800, 1280},   // WXGA tablet portrait
            // Older/smaller devices
            {480, 856},    // FWVGA variant
            {480, 854},    // FWVGA
            {480, 853},    // FWVGA variant
            {480, 800},    // WVGA
            {540, 960},    // qHD
            {600, 1024},   // WSVGA
            {640, 1136},   // iPhone 5
            {750, 1334},   // iPhone 6/7/8
            {320, 480},    // HVGA
            {240, 320},    // QVGA
            // Small icons/indicators (prefer square, then portrait)
            {28, 28},
            {36, 51}, {36, 50}, {50, 36},
            {30, 60}, {60, 30},
            {40, 45}, {45, 40},
            {56, 14}, {14, 56},
            {7, 112}, {112, 7},
            // MTK splash indicator assets
            {304, 52}, {304, 1},
            {218, 51},
            {169, 28}, {169, 1},
            {163, 29}, {163, 1},
            {138, 20}, {138, 2},
            {135, 24}, {135, 1},
            {108, 121},
            {102, 1},
            {84, 121},
            {63, 105},
            {57, 64},
            {48, 54},
            {45, 139}, {45, 64},
            {38, 54},
            {32, 105}, {32, 36},
            {30, 27},
            {27, 36},
            {23, 33},
            {34, 33},
            {15, 27},
            {10, 16}
        };

        // Check common resolutions first
        for (const auto& res : common_resolutions) {
            uint32_t width = res.first;
            uint32_t height = res.second;
            if (width * height * bpp == byte_size) {
                results.push_back({width, height});
            }
        }

        // If not found in common resolutions, try factorization
        // but prefer dimensions with reasonable aspect ratios (portrait phone screens)
        if (results.empty()) {
            std::vector<std::pair<uint32_t, uint32_t>> all_factors;

            for (uint32_t width = 1; (size_t)width * width <= pixel_count; width++) {
                if (pixel_count % width == 0) {
                    uint32_t height = (uint32_t)(pixel_count / width);
                    if ((size_t)height * width * bpp == byte_size) {
                        all_factors.push_back({width, height});
                        if (width != height) {
                            all_factors.push_back({height, width});
                        }
                    }
                }
            }

            // Sort by aspect ratio preference (portrait phone screens: 16:9 to 20:9)
            std::sort(all_factors.begin(), all_factors.end(),
                [](const std::pair<uint32_t, uint32_t>& a,
                   const std::pair<uint32_t, uint32_t>& b) {
                    float ratio_a = static_cast<float>(a.second) / a.first;
                    float ratio_b = static_cast<float>(b.second) / b.first;

                    auto score = [](float r) -> float {
                        if (r >= 1.5f && r <= 2.5f) return 1000.0f - std::abs(r - 2.0f);
                        if (r == 1.0f) return 100.0f;
                        if (r > 1.0f && r < 1.5f) return 50.0f;
                        if (r > 2.5f) return 10.0f;
                        return 1.0f;
                    };

                    return score(ratio_a) > score(ratio_b);
                });

            results = all_factors;
        }

        return results;
    }
};

// Exception class
class MtkLogoException : public std::runtime_error {
public:
    explicit MtkLogoException(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace mtklogo

#endif // MTKLOGO_H
