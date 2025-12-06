/**
 * @file splash.cpp
 * @brief OPPO/OnePlus/Realme Splash Image Implementation
 */

#include "splash.h"
#include "../external/lodepng/lodepng.h"
#include <fstream>
#include <cstring>
#include <iostream>

namespace bootmod {
namespace splash {

SplashImage::SplashImage() 
    : has_ddph_(false)
    , image_count_(0)
    , width_(0)
    , height_(0) {
    memset(&ddph_header_, 0, sizeof(ddph_header_));
    memset(&splash_header_, 0, sizeof(splash_header_));
}

SplashImage::~SplashImage() {
}

bool SplashImage::isValidSplash(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Check for OPPO_SPLASH magic at offset 0x4000
    file.seekg(OPPO_SPLASH_HDR_OFFSET);
    char magic[12];
    file.read(magic, 12);
    
    return (memcmp(magic, OPPO_SPLASH_MAGIC, 12) == 0);
}

bool SplashImage::load(const std::string& filename) {
    filename_ = filename;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    // Check for DDPH header
    file.seekg(DDPH_HDR_OFFSET);
    file.read(reinterpret_cast<char*>(&ddph_header_), sizeof(ddph_header_));
    has_ddph_ = (ddph_header_.magic == DDPH_MAGIC_V1);
    
    // Read OPPO_SPLASH header
    file.seekg(OPPO_SPLASH_HDR_OFFSET);
    file.read(reinterpret_cast<char*>(&splash_header_), sizeof(splash_header_));
    
    // Validate magic
    if (memcmp(splash_header_.magic, OPPO_SPLASH_MAGIC, 12) != 0) {
        std::cerr << "Invalid splash.img magic" << std::endl;
        return false;
    }
    
    image_count_ = splash_header_.imgnumber;
    width_ = splash_header_.width;
    height_ = splash_header_.height;
    
    // Read metadata
    file.seekg(METADATA_OFFSET);
    metadata_.resize(image_count_);
    for (uint32_t i = 0; i < image_count_; i++) {
        file.read(reinterpret_cast<char*>(&metadata_[i]), sizeof(splash_metadata_hdr_t));
    }
    
    // Read compressed image data
    image_data_.resize(image_count_);
    for (uint32_t i = 0; i < image_count_; i++) {
        file.seekg(DATA_OFFSET + metadata_[i].offset);
        image_data_[i].resize(metadata_[i].compsz);
        file.read(reinterpret_cast<char*>(image_data_[i].data()), metadata_[i].compsz);
    }
    
    return true;
}

bool SplashImage::save(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }
    
    // Write DDPH header if present
    if (has_ddph_) {
        file.seekp(DDPH_HDR_OFFSET);
        file.write(reinterpret_cast<const char*>(&ddph_header_), sizeof(ddph_header_));
    }
    
    // Recalculate offsets
    uint32_t current_offset = 0;
    for (uint32_t i = 0; i < image_count_; i++) {
        metadata_[i].offset = current_offset;
        metadata_[i].compsz = image_data_[i].size();
        current_offset += metadata_[i].compsz;
    }
    
    // Write OPPO_SPLASH header
    file.seekp(OPPO_SPLASH_HDR_OFFSET);
    file.write(reinterpret_cast<const char*>(&splash_header_), sizeof(splash_header_));
    
    // Write metadata
    file.seekp(METADATA_OFFSET);
    for (uint32_t i = 0; i < image_count_; i++) {
        file.write(reinterpret_cast<const char*>(&metadata_[i]), sizeof(splash_metadata_hdr_t));
    }
    
    // Write compressed image data
    for (uint32_t i = 0; i < image_count_; i++) {
        file.seekp(DATA_OFFSET + metadata_[i].offset);
        file.write(reinterpret_cast<const char*>(image_data_[i].data()), image_data_[i].size());
    }
    
    return true;
}

ImageInfo SplashImage::getImageInfo(uint32_t index) const {
    ImageInfo info;
    
    if (index >= image_count_) {
        return info;
    }
    
    info.name = std::string(reinterpret_cast<const char*>(metadata_[index].name));
    info.width = width_;
    info.height = height_;
    info.offset = metadata_[index].offset;
    info.compressed_size = metadata_[index].compsz;
    info.uncompressed_size = metadata_[index].realsz;
    
    return info;
}

bool SplashImage::decompressImage(uint32_t index, std::vector<uint8_t>& output) {
    if (index >= image_count_) {
        return false;
    }
    
    // Skip gzip header (10 bytes) and footer (8 bytes)
    const std::vector<uint8_t>& compressed = image_data_[index];
    if (compressed.size() < 18) {
        std::cerr << "Compressed data too small" << std::endl;
        return false;
    }
    
    const uint8_t* gzip_data = compressed.data() + 10;
    size_t gzip_size = compressed.size() - 18;
    
    // Decompress using lodepng inflate (raw deflate without zlib header)
    size_t output_size = 0;
    unsigned char* decompressed = nullptr;
    
    // Use lodepng_inflate which handles raw deflate stream
    LodePNGDecompressSettings settings;
    lodepng_decompress_settings_init(&settings);
    
    unsigned error = lodepng_inflate(&decompressed, &output_size, 
                                     gzip_data, gzip_size, &settings);
    
    if (error) {
        std::cerr << "Decompression failed: " << lodepng_error_text(error) << std::endl;
        return false;
    }
    
    output.assign(decompressed, decompressed + output_size);
    free(decompressed);
    
    return true;
}

bool SplashImage::compressImage(const std::vector<uint8_t>& input, 
                                std::vector<uint8_t>& output) {
    // Compress using lodepng deflate (raw deflate without zlib header)
    unsigned char* compressed = nullptr;
    size_t compressed_size = 0;
    
    LodePNGCompressSettings settings;
    lodepng_compress_settings_init(&settings);
    
    unsigned error = lodepng_deflate(&compressed, &compressed_size,
                                     input.data(), input.size(), &settings);
    
    if (error) {
        std::cerr << "Compression failed: " << lodepng_error_text(error) << std::endl;
        return false;
    }
    
    // Build gzip format: header + deflate data + footer
    output.clear();
    output.reserve(10 + compressed_size + 8);
    
    // Gzip header
    gzip_header_t header;
    header.id1 = 0x1f;
    header.id2 = 0x8b;
    header.cm = 0x08;
    header.flg = 0x00;
    header.mtime = 0;
    header.compression = 0;
    header.os = 0;
    
    output.insert(output.end(), 
                  reinterpret_cast<uint8_t*>(&header),
                  reinterpret_cast<uint8_t*>(&header) + 10);
    
    // Insert raw deflate data (no zlib wrapper to strip)
    output.insert(output.end(), compressed, compressed + compressed_size);
    
    // Calculate CRC32 and size for gzip footer
    uint32_t crc = lodepng_crc32(input.data(), input.size());
    uint32_t isize = static_cast<uint32_t>(input.size());
    
    output.insert(output.end(), 
                  reinterpret_cast<uint8_t*>(&crc),
                  reinterpret_cast<uint8_t*>(&crc) + 4);
    output.insert(output.end(),
                  reinterpret_cast<uint8_t*>(&isize),
                  reinterpret_cast<uint8_t*>(&isize) + 4);
    
    free(compressed);
    return true;
}

std::vector<uint8_t> SplashImage::getImageData(uint32_t index, 
                                                uint32_t& width, 
                                                uint32_t& height) {
    std::vector<uint8_t> bmp_data;
    
    if (!decompressImage(index, bmp_data)) {
        return std::vector<uint8_t>();
    }
    
    // BMP header is 54 bytes (14 + 40)
    if (bmp_data.size() < 54) {
        std::cerr << "Invalid BMP data" << std::endl;
        return std::vector<uint8_t>();
    }
    
    // Read width and height from BMP header
    width = *reinterpret_cast<uint32_t*>(&bmp_data[18]);
    height = *reinterpret_cast<uint32_t*>(&bmp_data[22]);
    
    return bmp_data;
}

bool SplashImage::extractImage(uint32_t index, const std::string& output_path) {
    if (index >= image_count_) {
        std::cerr << "Invalid image index: " << index << std::endl;
        return false;
    }
    
    // Decompress BMP data
    std::vector<uint8_t> bmp_data;
    if (!decompressImage(index, bmp_data)) {
        return false;
    }
    
    // Parse BMP and convert to PNG
    if (bmp_data.size() < 54) {
        std::cerr << "Invalid BMP data" << std::endl;
        return false;
    }
    
    uint32_t width = *reinterpret_cast<uint32_t*>(&bmp_data[18]);
    uint32_t height = *reinterpret_cast<uint32_t*>(&bmp_data[22]);
    uint16_t bpp = *reinterpret_cast<uint16_t*>(&bmp_data[28]);
    uint32_t offset = *reinterpret_cast<uint32_t*>(&bmp_data[10]);  // Pixel data offset
    
    std::vector<uint8_t> rgba(width * height * 4);
    
    if (bpp == 8) {
        // 8-bit indexed color - palette at offset 54, pixel data at 'offset'
        if (bmp_data.size() < offset || bmp_data.size() < 54 + 256 * 4) {
            std::cerr << "Invalid 8-bit BMP data" << std::endl;
            return false;
        }
        
        const uint8_t* palette = bmp_data.data() + 54;  // 256 colors * 4 bytes (BGRA)
        const uint8_t* bmp_pixels = bmp_data.data() + offset;
        int row_size = ((width + 3) / 4) * 4;  // Row size with padding
        
        for (uint32_t y = 0; y < height; y++) {
            const uint8_t* src_row = bmp_pixels + (height - 1 - y) * row_size;
            uint8_t* dst_row = rgba.data() + y * width * 4;
            
            for (uint32_t x = 0; x < width; x++) {
                uint8_t index = src_row[x];
                const uint8_t* color = palette + index * 4;
                dst_row[x * 4 + 0] = color[2];  // R = B
                dst_row[x * 4 + 1] = color[1];  // G = G
                dst_row[x * 4 + 2] = color[0];  // B = R
                dst_row[x * 4 + 3] = 255;       // A
            }
        }
    } else if (bpp == 24 || bpp == 32) {
        // 24-bit or 32-bit true color
        const uint8_t* bmp_pixels = bmp_data.data() + offset;
        int bytes_per_pixel = bpp / 8;
        int row_size = ((width * bytes_per_pixel + 3) / 4) * 4;  // Row size with padding
        
        for (uint32_t y = 0; y < height; y++) {
            const uint8_t* src_row = bmp_pixels + (height - 1 - y) * row_size;
            uint8_t* dst_row = rgba.data() + y * width * 4;
            
            for (uint32_t x = 0; x < width; x++) {
                dst_row[x * 4 + 0] = src_row[x * bytes_per_pixel + 2];  // R = B
                dst_row[x * 4 + 1] = src_row[x * bytes_per_pixel + 1];  // G = G
                dst_row[x * 4 + 2] = src_row[x * bytes_per_pixel + 0];  // B = R
                
                // Handle alpha: 32-bit BMPs store alpha, but BMP viewers typically ignore it
                // Splash screens are meant to be displayed opaque, so force alpha=255 for all pixels with alpha=0
                uint8_t alpha = (bpp == 32) ? src_row[x * bytes_per_pixel + 3] : 255;
                if (bpp == 32 && alpha == 0) {
                    alpha = 255;  // Force opaque - splash screens don't use transparency
                }
                dst_row[x * 4 + 3] = alpha;
            }
        }
    } else {
        std::cerr << "Unsupported BMP bit depth: " << bpp << "-bit (supported: 8, 24, 32)" << std::endl;
        return false;
    }
    
    // Encode to PNG
    unsigned error = lodepng::encode(output_path, rgba, width, height);
    if (error) {
        std::cerr << "PNG encoding failed: " << lodepng_error_text(error) << std::endl;
        return false;
    }
    
    return true;
}

bool SplashImage::replaceImage(uint32_t index, const std::string& png_path) {
    if (index >= image_count_) {
        std::cerr << "Invalid image index: " << index << std::endl;
        return false;
    }
    
    // Detect original BMP format (24-bit or 32-bit)
    std::vector<uint8_t> original_bmp;
    uint16_t original_bpp = 24;  // Default to 24-bit
    
    if (decompressImage(index, original_bmp) && original_bmp.size() >= 54) {
        original_bpp = *reinterpret_cast<uint16_t*>(&original_bmp[28]);
        std::cout << "  Original format: " << original_bpp << "-bit BMP\n";
    }
    
    // Load PNG
    std::vector<uint8_t> rgba;
    unsigned width, height;
    
    unsigned error = lodepng::decode(rgba, width, height, png_path);
    if (error) {
        std::cerr << "PNG decoding failed: " << lodepng_error_text(error) << std::endl;
        return false;
    }
    
    // Use original BMP format (preserve 32-bit if it was 32-bit)
    uint16_t bpp = original_bpp;
    int bytes_per_pixel = bpp / 8;
    int row_size = ((width * bytes_per_pixel + 3) / 4) * 4;
    std::vector<uint8_t> bmp_data(54 + row_size * height, 0);
    
    // BMP file header (14 bytes)
    bmp_data[0] = 'B';
    bmp_data[1] = 'M';
    uint32_t file_size = static_cast<uint32_t>(bmp_data.size());
    memcpy(&bmp_data[2], &file_size, 4);
    uint32_t offset = 54;
    memcpy(&bmp_data[10], &offset, 4);
    
    // BMP info header (40 bytes)
    uint32_t header_size = 40;
    memcpy(&bmp_data[14], &header_size, 4);
    memcpy(&bmp_data[18], &width, 4);
    memcpy(&bmp_data[22], &height, 4);
    uint16_t planes = 1;
    memcpy(&bmp_data[26], &planes, 2);
    memcpy(&bmp_data[28], &bpp, 2);
    
    // Convert RGBA top-down to BGR(A) bottom-up
    uint8_t* bmp_pixels = bmp_data.data() + 54;
    
    for (unsigned y = 0; y < height; y++) {
        const uint8_t* src_row = rgba.data() + y * width * 4;
        uint8_t* dst_row = bmp_pixels + (height - 1 - y) * row_size;
        
        for (unsigned x = 0; x < width; x++) {
            dst_row[x * bytes_per_pixel + 0] = src_row[x * 4 + 2];  // B = R
            dst_row[x * bytes_per_pixel + 1] = src_row[x * 4 + 1];  // G = G
            dst_row[x * bytes_per_pixel + 2] = src_row[x * 4 + 0];  // R = B
            if (bpp == 32) {
                dst_row[x * bytes_per_pixel + 3] = src_row[x * 4 + 3];  // A = A
            }
        }
    }
    
    // Compress BMP to gzip
    std::vector<uint8_t> compressed;
    if (!compressImage(bmp_data, compressed)) {
        return false;
    }
    
    // Update image data and metadata
    image_data_[index] = compressed;
    metadata_[index].compsz = compressed.size();
    metadata_[index].realsz = bmp_data.size();
    
    return true;
}

} // namespace splash
} // namespace bootmod
