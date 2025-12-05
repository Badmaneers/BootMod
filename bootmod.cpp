#include "bootmod.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <zlib.h>

namespace mtklogo {

// Helper: Read uint32 in Little Endian
static uint32_t readU32LE(const uint8_t* data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

// Helper: Read uint32 in Big Endian
static uint32_t readU32BE(const uint8_t* data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

// Helper: Write uint32 in Little Endian
static void writeU32LE(std::vector<uint8_t>& buffer, uint32_t value) {
    buffer.push_back(value & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 24) & 0xFF);
}

// Helper: Write uint32 in Big Endian
static void writeU32BE(std::vector<uint8_t>& buffer, uint32_t value) {
    buffer.push_back((value >> 24) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

// Helper: Case-insensitive comparison
static bool startsWithIgnoreCase(const uint8_t* data, const char* prefix, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = static_cast<char>(data[i]);
        if (std::tolower(c) != std::tolower(prefix[i])) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// MtkHeader Implementation
// ============================================================================

std::string MtkHeader::getTypeName() const {
    switch (mtk_type) {
        case MtkType::LOGO: return "LOGO";
        case MtkType::RECOVERY: return "RECOVERY";
        case MtkType::KERNEL: return "KERNEL";
        case MtkType::ROOTFS: return "ROOTFS";
        default: return "UNKNOWN";
    }
}

MtkHeader MtkHeader::read(const uint8_t* data, size_t length) {
    if (length < SIZE) {
        throw MtkLogoException("Buffer too small for MTK header");
    }

    MtkHeader header;
    
    // Read magic number (Big Endian)
    uint32_t magic = readU32BE(data);
    if (magic != MAGIC) {
        throw MtkLogoException("Invalid MTK magic number");
    }
    
    // Read size (Little Endian)
    header.size = readU32LE(data + 4);
    
    // Read type (32 bytes at offset 8)
    const uint8_t* type_data = data + 8;
    
    if (startsWithIgnoreCase(type_data, "LOGO", 4)) {
        header.mtk_type = MtkType::LOGO;
    } else if (startsWithIgnoreCase(type_data, "RECOVERY", 8)) {
        header.mtk_type = MtkType::RECOVERY;
    } else if (startsWithIgnoreCase(type_data, "KERNEL", 6)) {
        header.mtk_type = MtkType::KERNEL;
    } else if (startsWithIgnoreCase(type_data, "ROOTFS", 6)) {
        header.mtk_type = MtkType::ROOTFS;
    } else {
        throw MtkLogoException("Unknown MTK type");
    }
    
    return header;
}

void MtkHeader::write(std::vector<uint8_t>& buffer) const {
    size_t start_size = buffer.size();
    
    // Write magic (Big Endian)
    writeU32BE(buffer, MAGIC);
    
    // Write size (Little Endian)
    writeU32LE(buffer, size);
    
    // Write type (32 bytes)
    std::string type_name = getTypeName();
    for (size_t i = 0; i < 32; i++) {
        if (i < type_name.length()) {
            buffer.push_back(type_name[i]);
        } else {
            buffer.push_back(0);
        }
    }
    
    // Fill remainder with 0xFF (472 bytes)
    for (size_t i = 0; i < 472; i++) {
        buffer.push_back(FILL);
    }
}

// ============================================================================
// LogoTable Implementation
// ============================================================================

LogoTable LogoTable::read(const uint8_t* data, size_t length) {
    if (length < MtkHeader::SIZE) {
        throw MtkLogoException("Buffer too small for logo table");
    }
    
    LogoTable table;
    
    // Read MTK header
    table.header = MtkHeader::read(data, length);
    
    if (table.header.mtk_type != MtkType::LOGO) {
        throw MtkLogoException("MTK image is not a logo");
    }
    
    // Read logo count and block size
    const uint8_t* table_data = data + MtkHeader::SIZE;
    size_t remaining = length - MtkHeader::SIZE;
    
    if (remaining < 8) {
        throw MtkLogoException("Buffer too small for logo table data");
    }
    
    table.logo_count = readU32LE(table_data);
    table.block_size = readU32LE(table_data + 4);
    
    if (table.block_size != table.header.size) {
        throw MtkLogoException("MTK Header size does not match block size");
    }
    
    // Read offsets
    const uint8_t* offset_data = table_data + 8;
    if (remaining < 8 + table.logo_count * 4) {
        throw MtkLogoException("Buffer too small for all offsets");
    }
    
    table.offsets.reserve(table.logo_count);
    for (uint32_t i = 0; i < table.logo_count; i++) {
        uint32_t offset = readU32LE(offset_data + i * 4);
        table.offsets.push_back(offset);
    }
    
    return table;
}

void LogoTable::write(std::vector<uint8_t>& buffer) const {
    // Write header
    header.write(buffer);
    
    // Write logo count
    writeU32LE(buffer, logo_count);
    
    // Write block size
    writeU32LE(buffer, block_size);
    
    // Write offsets
    for (uint32_t offset : offsets) {
        writeU32LE(buffer, offset);
    }
}

uint32_t LogoTable::getBlobSize(size_t index) const {
    if (index >= offsets.size()) {
        throw MtkLogoException("Blob index out of range");
    }
    
    uint32_t offset = offsets[index];
    uint32_t next_offset = (index < offsets.size() - 1) ? offsets[index + 1] : block_size;
    
    return next_offset - offset;
}

// ============================================================================
// LogoImage Implementation
// ============================================================================

LogoImage LogoImage::readFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw MtkLogoException("Cannot open file: " + filename);
    }
    
    // Read entire file
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    if (!file) {
        throw MtkLogoException("Failed to read file: " + filename);
    }
    
    return read(buffer.data(), size);
}

LogoImage LogoImage::read(const uint8_t* data, size_t length) {
    LogoImage image;
    
    // Read logo table
    image.table = LogoTable::read(data, length);
    
    // Read blobs
    const uint8_t* blob_data = data + MtkHeader::SIZE;
    
    image.blobs.reserve(image.table.logo_count);
    for (size_t i = 0; i < image.table.logo_count; i++) {
        uint32_t offset = image.table.offsets[i];
        uint32_t size = image.table.getBlobSize(i);
        
        if (MtkHeader::SIZE + offset + size > length) {
            throw MtkLogoException("Blob exceeds buffer size");
        }
        
        const uint8_t* blob_start = blob_data + offset;
        std::vector<uint8_t> blob(blob_start, blob_start + size);
        image.blobs.push_back(std::move(blob));
    }
    
    return image;
}

LogoImage LogoImage::createFromBlobs(const std::vector<std::vector<uint8_t>>& blobs) {
    LogoImage image;
    image.blobs = blobs;
    
    // Calculate offsets
    uint32_t offset = (2 + blobs.size()) * 4; // Start after table
    
    image.table.offsets.reserve(blobs.size());
    for (const auto& blob : blobs) {
        image.table.offsets.push_back(offset);
        offset += blob.size();
    }
    
    image.table.block_size = offset;
    image.table.logo_count = blobs.size();
    image.table.header.size = offset;
    image.table.header.mtk_type = MtkType::LOGO;
    
    return image;
}

void LogoImage::writeToFile(const std::string& filename) const {
    std::vector<uint8_t> buffer = write();
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw MtkLogoException("Cannot create file: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    
    if (!file) {
        throw MtkLogoException("Failed to write file: " + filename);
    }
}

std::vector<uint8_t> LogoImage::write() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(MtkHeader::SIZE + table.block_size);
    
    // Write table
    table.write(buffer);
    
    // Write blobs
    for (const auto& blob : blobs) {
        buffer.insert(buffer.end(), blob.begin(), blob.end());
    }
    
    return buffer;
}

// ============================================================================
// ImageUtils Implementation
// ============================================================================

std::vector<uint8_t> ImageUtils::zlibDecompress(const std::vector<uint8_t>& compressed) {
    z_stream stream = {};
    stream.next_in = const_cast<uint8_t*>(compressed.data());
    stream.avail_in = compressed.size();
    
    if (inflateInit(&stream) != Z_OK) {
        throw MtkLogoException("Failed to initialize zlib inflation");
    }
    
    std::vector<uint8_t> decompressed;
    const size_t CHUNK_SIZE = 16384;
    
    int ret;
    do {
        uint8_t out_buffer[CHUNK_SIZE];
        stream.next_out = out_buffer;
        stream.avail_out = CHUNK_SIZE;
        
        ret = inflate(&stream, Z_NO_FLUSH);
        
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&stream);
            throw MtkLogoException("Zlib decompression failed");
        }
        
        size_t have = CHUNK_SIZE - stream.avail_out;
        decompressed.insert(decompressed.end(), out_buffer, out_buffer + have);
        
    } while (ret != Z_STREAM_END);
    
    inflateEnd(&stream);
    return decompressed;
}

std::vector<uint8_t> ImageUtils::zlibCompress(const std::vector<uint8_t>& data, int level) {
    z_stream stream = {};
    stream.next_in = const_cast<uint8_t*>(data.data());
    stream.avail_in = data.size();
    
    if (deflateInit(&stream, level) != Z_OK) {
        throw MtkLogoException("Failed to initialize zlib deflation");
    }
    
    std::vector<uint8_t> compressed;
    const size_t CHUNK_SIZE = 16384;
    
    int ret;
    do {
        uint8_t out_buffer[CHUNK_SIZE];
        stream.next_out = out_buffer;
        stream.avail_out = CHUNK_SIZE;
        
        ret = deflate(&stream, Z_FINISH);
        
        if (ret != Z_OK && ret != Z_STREAM_END) {
            deflateEnd(&stream);
            throw MtkLogoException("Zlib compression failed");
        }
        
        size_t have = CHUNK_SIZE - stream.avail_out;
        compressed.insert(compressed.end(), out_buffer, out_buffer + have);
        
    } while (ret != Z_STREAM_END);
    
    deflateEnd(&stream);
    return compressed;
}

uint32_t ImageUtils::getBytesPerPixel(ColorMode mode) {
    switch (mode) {
        case ColorMode::RGBA_BE:
        case ColorMode::RGBA_LE:
        case ColorMode::BGRA_BE:
        case ColorMode::BGRA_LE:
            return 4;
        case ColorMode::RGB565_BE:
        case ColorMode::RGB565_LE:
            return 2;
        default:
            return 4;
    }
}

std::string ImageUtils::getColorModeName(ColorMode mode) {
    switch (mode) {
        case ColorMode::RGBA_BE: return "rgbabe";
        case ColorMode::RGBA_LE: return "rgbale";
        case ColorMode::BGRA_BE: return "bgrabe";
        case ColorMode::BGRA_LE: return "bgrale";
        case ColorMode::RGB565_BE: return "rgb565be";
        case ColorMode::RGB565_LE: return "rgb565le";
        default: return "unknown";
    }
}

ColorMode ImageUtils::parseColorMode(const std::string& name) {
    if (name == "rgbabe") return ColorMode::RGBA_BE;
    if (name == "rgbale") return ColorMode::RGBA_LE;
    if (name == "bgrabe") return ColorMode::BGRA_BE;
    if (name == "bgrale") return ColorMode::BGRA_LE;
    if (name == "rgb565be") return ColorMode::RGB565_BE;
    if (name == "rgb565le") return ColorMode::RGB565_LE;
    
    throw MtkLogoException("Unknown color mode: " + name);
}

// ============================================================================
// FileInfo Implementation
// ============================================================================

std::string FileInfo::getFilename() const {
    std::ostringstream oss;
    oss << "logo_" << std::setfill('0') << std::setw(3) << id << "_";
    
    if (is_compressed) {
        oss << "raw.z";
    } else {
        oss << ImageUtils::getColorModeName(color_mode) << ".png";
    }
    
    return oss.str();
}

FileInfo FileInfo::fromFilename(const std::string& filename) {
    // Parse "logo_XXX_type.ext" format
    size_t first_underscore = filename.find('_');
    size_t second_underscore = filename.find('_', first_underscore + 1);
    
    if (first_underscore == std::string::npos || second_underscore == std::string::npos) {
        throw MtkLogoException("Invalid filename format");
    }
    
    FileInfo info;
    
    // Extract ID
    std::string id_str = filename.substr(first_underscore + 1, second_underscore - first_underscore - 1);
    info.id = std::stoul(id_str);
    
    // Check if compressed
    info.is_compressed = filename.find("raw.z") != std::string::npos;
    
    if (!info.is_compressed) {
        // Extract color mode from filename
        size_t dot_pos = filename.rfind('.');
        if (dot_pos == std::string::npos) {
            throw MtkLogoException("Invalid filename: no extension");
        }
        
        std::string mode_name = filename.substr(second_underscore + 1, dot_pos - second_underscore - 1);
        info.color_mode = ImageUtils::parseColorMode(mode_name);
    }
    
    return info;
}

} // namespace mtklogo
