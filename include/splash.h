/**
 * @file splash.h
 * @brief OPPO/OnePlus/Realme Splash Image Support
 * @author Badmaneers
 * 
 * Support for Snapdragon-based devices using splash.img format
 * Based on opsplash by affggh
 */

#ifndef BOOTMOD_SPLASH_H
#define BOOTMOD_SPLASH_H

#include <stdint.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

// DDPH Header (optional, at offset 0x0)
#define DDPH_MAGIC_V1 0x48504444
#define DDPH_HDR_OFFSET 0x0

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t flag;
} ddph_hdr_t;

// OPPO Splash Header (at offset 0x4000)
#define OPPO_SPLASH_MAGIC "SPLASH LOGO!"
#define OPPO_SPLASH_HDR_OFFSET 0x4000

typedef struct {
    uint8_t magic[12];              // "SPLASH LOGO!"
    uint8_t metadata[3][0x40];      // 3 metadata strings
    uint8_t zerofill[0x40];         // Zero padding
    uint32_t imgnumber;             // Number of images
    uint32_t unknown;               // Unknown flag
    uint32_t width;                 // Display width
    uint32_t height;                // Display height
    uint32_t special;               // Special flags
} oppo_splash_hdr_t;

// Metadata offset calculation
#define METADATA_OFFSET (sizeof(oppo_splash_hdr_t) + OPPO_SPLASH_HDR_OFFSET)

typedef struct {
    uint32_t offset;                // Offset from DATA_OFFSET
    uint32_t realsz;                // Uncompressed size
    uint32_t compsz;                // Compressed size
    uint8_t name[0x74];             // Image name (116 bytes)
} splash_metadata_hdr_t;

// Compressed data starts at offset 0x8000
#define DATA_OFFSET 0x8000

// Gzip header structure
typedef struct {
    uint8_t id1;                    // 0x1f
    uint8_t id2;                    // 0x8b
    uint8_t cm;                     // 0x08 (deflate)
    uint8_t flg;                    // Flags
    uint32_t mtime;                 // Modification time
    uint8_t compression;            // Extra flags
    uint8_t os;                     // Operating system
} gzip_header_t;

typedef struct {
    uint32_t crc32;                 // CRC32 checksum
    uint32_t isize;                 // Original size
} gzip_footer_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

namespace bootmod {
namespace splash {

/**
 * @brief Image information structure
 */
struct ImageInfo {
    std::string name;
    uint32_t width;
    uint32_t height;
    uint32_t offset;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
};

/**
 * @brief Splash image class for OPPO/OnePlus devices
 */
class SplashImage {
public:
    SplashImage();
    ~SplashImage();
    
    /**
     * @brief Load splash.img file
     * @param filename Path to splash.img
     * @return true if successful
     */
    bool load(const std::string& filename);
    
    /**
     * @brief Save splash.img file
     * @param filename Output path
     * @return true if successful
     */
    bool save(const std::string& filename);
    
    /**
     * @brief Get number of images
     */
    uint32_t getImageCount() const { return image_count_; }
    
    /**
     * @brief Get display width
     */
    uint32_t getWidth() const { return width_; }
    
    /**
     * @brief Get display height
     */
    uint32_t getHeight() const { return height_; }
    
    /**
     * @brief Get image information
     * @param index Image index (0-based)
     */
    ImageInfo getImageInfo(uint32_t index) const;
    
    /**
     * @brief Get display resolution
     */
    void getResolution(uint32_t& width, uint32_t& height) const {
        width = width_;
        height = height_;
    }
    
    /**
     * @brief Extract image as PNG
     * @param index Image index
     * @param output_path Output PNG file path
     * @return true if successful
     */
    bool extractImage(uint32_t index, const std::string& output_path);
    
    /**
     * @brief Replace image from PNG
     * @param index Image index
     * @param png_path Input PNG file path
     * @return true if successful
     */
    bool replaceImage(uint32_t index, const std::string& png_path);
    
    /**
     * @brief Get raw BMP data for an image
     * @param index Image index
     * @param width Output width
     * @param height Output height
     * @return BMP data
     */
    std::vector<uint8_t> getImageData(uint32_t index, uint32_t& width, uint32_t& height);
    
    /**
     * @brief Check if file is a valid splash.img
     * @param filename Path to check
     * @return true if valid
     */
    static bool isValidSplash(const std::string& filename);
    
private:
    bool parseHeader();
    bool decompressImage(uint32_t index, std::vector<uint8_t>& output);
    bool compressImage(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
    
    std::string filename_;
    ddph_hdr_t ddph_header_;
    oppo_splash_hdr_t splash_header_;
    std::vector<splash_metadata_hdr_t> metadata_;
    
    bool has_ddph_;
    uint32_t image_count_;
    uint32_t width_;
    uint32_t height_;
    
    std::vector<std::vector<uint8_t>> image_data_;  // Cached compressed data
};

} // namespace splash
} // namespace bootmod

#endif // BOOTMOD_SPLASH_H
