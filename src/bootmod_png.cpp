#include "bootmod.h"
#include <png.h>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace mtklogo {

// ============================================================================
// PNG I/O Implementation
// ============================================================================

bool ImageUtils::saveToPNG(const std::string& filename, 
                          const std::vector<uint8_t>& pixels,
                          uint32_t width, 
                          uint32_t height,
                          ColorMode mode) {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        return false;
    }
    
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        return false;
    }
    
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        return false;
    }
    
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return false;
    }
    
    png_init_io(png, fp);
    
    // Convert to RGBA for PNG output
    std::vector<uint8_t> rgba_pixels;
    uint32_t bpp = getBytesPerPixel(mode);
    
    if (mode == ColorMode::RGBA_BE || mode == ColorMode::RGBA_LE) {
        // Already RGBA, might need endian swap
        rgba_pixels = pixels;
        if (mode == ColorMode::RGBA_BE) {
            // Swap endianness if needed
            for (size_t i = 0; i < rgba_pixels.size(); i += 4) {
                // For RGBA_BE, swap if needed (typically we want LE for PNG)
            }
        }
    } else if (mode == ColorMode::BGRA_BE || mode == ColorMode::BGRA_LE) {
        // Convert BGRA to RGBA
        rgba_pixels.reserve(pixels.size());
        for (size_t i = 0; i < pixels.size(); i += 4) {
            rgba_pixels.push_back(pixels[i + 2]); // R
            rgba_pixels.push_back(pixels[i + 1]); // G
            rgba_pixels.push_back(pixels[i + 0]); // B
            rgba_pixels.push_back(pixels[i + 3]); // A
        }
    } else if (mode == ColorMode::RGB565_BE || mode == ColorMode::RGB565_LE) {
        // Convert RGB565 to RGBA
        rgba_pixels.reserve(width * height * 4);
        for (size_t i = 0; i < pixels.size(); i += 2) {
            uint16_t pixel;
            if (mode == ColorMode::RGB565_LE) {
                pixel = pixels[i] | (pixels[i + 1] << 8);
            } else {
                pixel = (pixels[i] << 8) | pixels[i + 1];
            }
            
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
            uint8_t b = (pixel & 0x1F) << 3;
            
            rgba_pixels.push_back(r);
            rgba_pixels.push_back(g);
            rgba_pixels.push_back(b);
            rgba_pixels.push_back(255); // Full alpha
        }
    }
    
    // Set PNG header
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png, info);
    
    // Write image data
    std::vector<png_bytep> row_pointers(height);
    for (uint32_t y = 0; y < height; y++) {
        row_pointers[y] = const_cast<png_bytep>(&rgba_pixels[y * width * 4]);
    }
    
    png_write_image(png, row_pointers.data());
    png_write_end(png, nullptr);
    
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    
    return true;
}

std::vector<uint8_t> ImageUtils::loadFromPNG(const std::string& filename,
                                            uint32_t& width,
                                            uint32_t& height,
                                            ColorMode mode) {
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        throw MtkLogoException("Cannot open PNG file: " + filename);
    }
    
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        throw MtkLogoException("Failed to create PNG read struct");
    }
    
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        throw MtkLogoException("Failed to create PNG info struct");
    }
    
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        throw MtkLogoException("Error reading PNG file");
    }
    
    png_init_io(png, fp);
    png_read_info(png, info);
    
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    
    // Convert to RGBA
    if (bit_depth == 16) {
        png_set_strip_16(png);
    }
    
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }
    
    png_read_update_info(png, info);
    
    // Read image data
    std::vector<uint8_t> rgba_data(width * height * 4);
    std::vector<png_bytep> row_pointers(height);
    
    for (uint32_t y = 0; y < height; y++) {
        row_pointers[y] = &rgba_data[y * width * 4];
    }
    
    png_read_image(png, row_pointers.data());
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);
    
    // Convert RGBA to target color mode
    return convertColorMode(rgba_data, ColorMode::RGBA_LE, mode);
}

std::vector<uint8_t> ImageUtils::convertColorMode(const std::vector<uint8_t>& pixels,
                                                  ColorMode from,
                                                  ColorMode to) {
    if (from == to) {
        return pixels;
    }
    
    // For now, assume input is RGBA_LE and convert to target
    std::vector<uint8_t> output;
    
    if (to == ColorMode::BGRA_BE || to == ColorMode::BGRA_LE) {
        // Convert RGBA to BGRA
        output.reserve(pixels.size());
        for (size_t i = 0; i < pixels.size(); i += 4) {
            output.push_back(pixels[i + 2]); // B
            output.push_back(pixels[i + 1]); // G
            output.push_back(pixels[i + 0]); // R
            output.push_back(pixels[i + 3]); // A
        }
    } else if (to == ColorMode::RGB565_BE || to == ColorMode::RGB565_LE) {
        // Convert RGBA to RGB565
        output.reserve(pixels.size() / 2);
        for (size_t i = 0; i < pixels.size(); i += 4) {
            uint8_t r = pixels[i] >> 3;
            uint8_t g = pixels[i + 1] >> 2;
            uint8_t b = pixels[i + 2] >> 3;
            
            uint16_t pixel = (r << 11) | (g << 5) | b;
            
            if (to == ColorMode::RGB565_LE) {
                output.push_back(pixel & 0xFF);
                output.push_back(pixel >> 8);
            } else {
                output.push_back(pixel >> 8);
                output.push_back(pixel & 0xFF);
            }
        }
    } else {
        // RGBA_BE or RGBA_LE
        output = pixels;
    }
    
    return output;
}

// ============================================================================
// MtkLogo Main Operations
// ============================================================================

bool MtkLogo::unpack(const std::string& logo_file,
                    const std::string& output_dir,
                    ColorMode mode,
                    const std::vector<size_t>& slots,
                    bool extract_raw,
                    bool flip) {
    try {
        // Read logo image
        LogoImage image = LogoImage::readFromFile(logo_file);
        
        printf("Logo file: %s\n", logo_file.c_str());
        printf("Number of logos: %zu\n", image.getLogoCount());
        printf("Block size: %u bytes\n", image.table.block_size);
        
        // Process each logo
        for (size_t i = 0; i < image.getLogoCount(); i++) {
            // Check if we should extract this slot (slots are 1-based from user input)
            if (!slots.empty()) {
                if (std::find(slots.begin(), slots.end(), i + 1) == slots.end()) {
                    continue;
                }
            }
            
            const auto& blob = image.blobs[i];
            printf("\nProcessing logo %zu: %zu bytes\n", i + 1, blob.size());
            
            FileInfo file_info;
            file_info.id = i + 1;
            
            if (extract_raw) {
                // Save as raw .z file
                file_info.is_compressed = true;
                std::string output_path = output_dir + "/" + file_info.getFilename();
                
                std::ofstream out(output_path, std::ios::binary);
                out.write(reinterpret_cast<const char*>(blob.data()), blob.size());
                printf("  Saved: %s\n", output_path.c_str());
            } else {
                // Try to decompress and save as PNG
                try {
                    std::vector<uint8_t> decompressed = ImageUtils::zlibDecompress(blob);
                    printf("  Decompressed: %zu bytes\n", decompressed.size());
                    
                    // Try to guess dimensions
                    uint32_t bpp = ImageUtils::getBytesPerPixel(mode);
                    auto dimensions = guessDimensions(decompressed.size(), mode);
                    
                    if (!dimensions.empty()) {
                        uint32_t width = dimensions[0].first;
                        uint32_t height = dimensions[0].second;
                        
                        printf("  Dimensions: %ux%u\n", width, height);
                        
                        file_info.is_compressed = false;
                        file_info.color_mode = mode;
                        std::string output_path = output_dir + "/" + file_info.getFilename();
                        
                        if (ImageUtils::saveToPNG(output_path, decompressed, width, height, mode)) {
                            printf("  Saved: %s\n", output_path.c_str());
                        } else {
                            printf("  Failed to save PNG\n");
                        }
                    } else {
                        printf("  Could not determine dimensions, saving as raw\n");
                        file_info.is_compressed = true;
                        std::string output_path = output_dir + "/" + file_info.getFilename();
                        
                        std::ofstream out(output_path, std::ios::binary);
                        out.write(reinterpret_cast<const char*>(blob.data()), blob.size());
                    }
                } catch (const std::exception& e) {
                    printf("  Not compressed or decompression failed, saving as raw\n");
                    file_info.is_compressed = true;
                    std::string output_path = output_dir + "/" + file_info.getFilename();
                    
                    std::ofstream out(output_path, std::ios::binary);
                    out.write(reinterpret_cast<const char*>(blob.data()), blob.size());
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return false;
    }
}

bool MtkLogo::repack(const std::vector<std::string>& input_files,
                    const std::string& output_file,
                    bool strip_alpha) {
    try {
        printf("Repacking %zu files into %s\n", input_files.size(), output_file.c_str());
        
        std::vector<std::pair<size_t, std::string>> sorted_files;
        
        // Parse and sort files by ID
        for (const auto& file : input_files) {
            size_t last_slash = file.find_last_of("/\\");
            std::string filename = (last_slash != std::string::npos) ? 
                                  file.substr(last_slash + 1) : file;
            
            FileInfo info = FileInfo::fromFilename(filename);
            sorted_files.push_back({info.id, file});
        }
        
        std::sort(sorted_files.begin(), sorted_files.end());
        
        // Create blobs
        std::vector<std::vector<uint8_t>> blobs;
        
        for (const auto& pair : sorted_files) {
            size_t id = pair.first;
            const std::string& file = pair.second;
            
            printf("Processing file %zu: %s\n", id, file.c_str());
            
            size_t last_slash = file.find_last_of("/\\");
            std::string filename = (last_slash != std::string::npos) ? 
                                  file.substr(last_slash + 1) : file;
            
            FileInfo info = FileInfo::fromFilename(filename);
            
            if (info.is_compressed) {
                // Read raw file
                std::ifstream in(file, std::ios::binary);
                std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),
                                         std::istreambuf_iterator<char>());
                blobs.push_back(data);
            } else {
                // Load PNG and convert
                uint32_t width, height;
                std::vector<uint8_t> pixels = ImageUtils::loadFromPNG(file, width, height, info.color_mode);
                
                printf("  Loaded PNG: %ux%u, %zu bytes\n", width, height, pixels.size());
                
                // Compress
                std::vector<uint8_t> compressed = ImageUtils::zlibCompress(pixels);
                printf("  Compressed: %zu bytes\n", compressed.size());
                
                blobs.push_back(compressed);
            }
        }
        
        // Create and write logo image
        LogoImage image = LogoImage::createFromBlobs(blobs);
        image.writeToFile(output_file);
        
        printf("\nSuccessfully created: %s\n", output_file.c_str());
        printf("Total size: %zu bytes\n", image.table.block_size + MtkHeader::SIZE);
        
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return false;
    }
}

} // namespace mtklogo
