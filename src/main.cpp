#include "bootmod.h"
#include "splash.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <filesystem>

#ifndef VERSION
#define VERSION "1.6.0"
#endif

namespace fs = std::filesystem;

void printUsage(const char* program) {
    std::cout << "BootMod - Universal Boot Logo/Splash Editor v" << VERSION << "\n";
    std::cout << "Supports: MediaTek logo.bin, Qualcomm splash.img\n\n";
    std::cout << "Usage: " << program << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  unpack <logo.bin|splash.img> <output_dir> [options]\n";
    std::cout << "    Extract logos from boot image file\n";
    std::cout << "    Options (MTK only):\n";
    std::cout << "      --mode <mode>        Color mode (bgrabe, bgrale, rgbabe, rgbale, rgb565be, rgb565le)\n";
    std::cout << "      --slots <0,1,2>      Extract only specific slots (comma-separated)\n";
    std::cout << "      --raw                Extract as raw .z files without decompression\n";
    std::cout << "      --flip               Flip orientation\n";
    std::cout << "\n";
    std::cout << "  repack <output.bin> <file1> <file2> ... [options]\n";
    std::cout << "    Repack logo files into logo.bin (MTK format)\n";
    std::cout << "    Options:\n";
    std::cout << "      --strip-alpha        Remove alpha channel\n";
    std::cout << "\n";
    std::cout << "  info <logo.bin|splash.img>\n";
    std::cout << "    Display information about boot image file\n";
    std::cout << "\n";
    std::cout << "  extract <splash.img> <index> <output.png>\n";
    std::cout << "    Extract single image from splash.img (Snapdragon)\n";
    std::cout << "\n";
    std::cout << "  replace <splash.img> <index> <input.png> <output.img>\n";
    std::cout << "    Replace single image in splash.img (Snapdragon)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program << " unpack logo.bin extracted/\n";
    std::cout << "  " << program << " unpack splash.img extracted/\n";
    std::cout << "  " << program << " repack new_logo.bin extracted/logo_*.png\n";
    std::cout << "  " << program << " extract splash.img 0 logo.png\n";
    std::cout << "  " << program << " replace splash.img 0 new_logo.png output.img\n";
    std::cout << "  " << program << " info logo.bin\n";
}

std::vector<size_t> parseSlots(const std::string& slots_str) {
    std::vector<size_t> slots;
    size_t start = 0;
    size_t end = slots_str.find(',');
    
    while (end != std::string::npos) {
        slots.push_back(std::stoul(slots_str.substr(start, end - start)));
        start = end + 1;
        end = slots_str.find(',', start);
    }
    slots.push_back(std::stoul(slots_str.substr(start)));
    
    return slots;
}

int cmdUnpack(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: unpack requires <logo.bin|splash.img> <output_dir>\n";
        return 1;
    }
    
    std::string input_file = argv[2];
    std::string output_dir = argv[3];
    
    // Detect format
    bootmod::FormatType format = bootmod::detectFormat(input_file);
    
    if (format == bootmod::FormatType::OPPO_SPLASH) {
        // Unpack Snapdragon splash.img
        bootmod::splash::SplashImage splash;
        if (!splash.load(input_file)) {
            std::cerr << "Failed to load splash.img\n";
            return 1;
        }
        
        // Create output directory
        fs::create_directories(output_dir);
        
        std::cout << "Unpacking Snapdragon splash.img...\n";
        std::cout << "Image count: " << splash.getImageCount() << "\n";
        
        uint32_t width, height;
        splash.getResolution(width, height);
        std::cout << "Resolution: " << width << "x" << height << "\n\n";
        
        for (uint32_t i = 0; i < splash.getImageCount(); i++) {
            auto info = splash.getImageInfo(i);
            std::string output_path = output_dir + "/image_" + std::to_string(i) + ".png";
            
            std::cout << "Extracting image " << i << ": " << info.name << " -> " << output_path << "\n";
            if (!splash.extractImage(i, output_path)) {
                std::cerr << "Failed to extract image " << i << "\n";
                return 1;
            }
        }
        
        std::cout << "\nExtraction complete!\n";
        return 0;
        
    } else if (format == bootmod::FormatType::MTK_LOGO) {
        // Unpack MediaTek logo.bin
        mtklogo::ColorMode mode = mtklogo::ColorMode::BGRA_BE;
        std::vector<size_t> slots;
        bool extract_raw = false;
        bool flip = false;
        
        // Parse options
        for (int i = 4; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "--mode" && i + 1 < argc) {
                mode = mtklogo::ImageUtils::parseColorMode(argv[++i]);
            } else if (arg == "--slots" && i + 1 < argc) {
                slots = parseSlots(argv[++i]);
            } else if (arg == "--raw") {
                extract_raw = true;
            } else if (arg == "--flip") {
                flip = true;
            }
        }
        
        return mtklogo::MtkLogo::unpack(input_file, output_dir, mode, slots, extract_raw, flip) ? 0 : 1;
        
    } else {
        std::cerr << "Error: Unknown file format (not MTK logo.bin or OPPO splash.img)\n";
        return 1;
    }
}

int cmdRepack(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: repack requires <output.bin> <file1> [file2] ...\n";
        return 1;
    }
    
    std::string output_file = argv[2];
    std::vector<std::string> input_files;
    bool strip_alpha = false;
    
    // Parse files and options
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--strip-alpha") {
            strip_alpha = true;
        } else {
            input_files.push_back(arg);
        }
    }
    
    if (input_files.empty()) {
        std::cerr << "Error: No input files specified\n";
        return 1;
    }
    
    return mtklogo::MtkLogo::repack(input_files, output_file, strip_alpha) ? 0 : 1;
}

int cmdInfo(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: info requires <logo.bin|splash.img>\n";
        return 1;
    }
    
    std::string input_file = argv[2];
    bootmod::FormatType format = bootmod::detectFormat(input_file);
    
    if (format == bootmod::FormatType::OPPO_SPLASH) {
        // Show Snapdragon splash.img info
        bootmod::splash::SplashImage splash;
        if (!splash.load(input_file)) {
            std::cerr << "Failed to load splash.img\n";
            return 1;
        }
        
        uint32_t width, height;
        splash.getResolution(width, height);
        
        std::cout << "Snapdragon Splash Information\n";
        std::cout << "=============================\n";
        std::cout << "File: " << input_file << "\n";
        std::cout << "Format: OPPO/OnePlus splash.img\n";
        std::cout << "Resolution: " << width << "x" << height << "\n";
        std::cout << "Number of images: " << splash.getImageCount() << "\n\n";
        std::cout << "Image Details:\n";
        
        for (uint32_t i = 0; i < splash.getImageCount(); i++) {
            auto info = splash.getImageInfo(i);
            std::cout << "  Image " << i << ": " << info.name << "\n";
            std::cout << "    Offset: 0x" << std::hex << info.offset << std::dec << "\n";
            std::cout << "    Compressed size: " << info.compressed_size << " bytes\n";
            std::cout << "    Uncompressed size: " << info.uncompressed_size << " bytes\n";
        }
        
        return 0;
        
    } else if (format == bootmod::FormatType::MTK_LOGO) {
        // Show MediaTek logo.bin info
        try {
            mtklogo::LogoImage image = mtklogo::LogoImage::readFromFile(input_file);
            
            std::cout << "MTK Logo Information\n";
            std::cout << "====================\n";
            std::cout << "File: " << input_file << "\n";
            std::cout << "Type: " << image.table.header.getTypeName() << "\n";
            std::cout << "Number of logos: " << image.table.logo_count << "\n";
            std::cout << "Block size: " << image.table.block_size << " bytes\n";
            std::cout << "Total size: " << (mtklogo::MtkHeader::SIZE + image.table.block_size) << " bytes\n";
            std::cout << "\nLogo Details:\n";
            
            for (size_t i = 0; i < image.getLogoCount(); i++) {
                uint32_t size = image.table.getBlobSize(i);
                std::cout << "  Logo " << (i + 1) << ": " << size << " bytes";
                
                // Try to detect if compressed
                const auto& blob = image.blobs[i];
                if (blob.size() >= 2) {
                    // Check for zlib header
                    if ((blob[0] == 0x78 && (blob[1] == 0x01 || blob[1] == 0x9C || blob[1] == 0xDA))) {
                        std::cout << " (zlib compressed)";
                        
                        try {
                            auto decompressed = mtklogo::ImageUtils::zlibDecompress(blob);
                            std::cout << " -> " << decompressed.size() << " bytes decompressed";
                        } catch (...) {
                            std::cout << " (decompression failed)";
                        }
                    }
                }
                
                std::cout << "\n";
            }
            
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
        
    } else {
        std::cerr << "Error: Unknown file format\n";
        return 1;
    }
}

int cmdExtract(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Error: extract requires <splash.img> <index> <output.png>\n";
        return 1;
    }
    
    std::string input_file = argv[2];
    int index = std::stoi(argv[3]);
    std::string output_file = argv[4];
    
    bootmod::splash::SplashImage splash;
    if (!splash.load(input_file)) {
        std::cerr << "Failed to load splash.img\n";
        return 1;
    }
    
    if (index < 0 || index >= static_cast<int>(splash.getImageCount())) {
        std::cerr << "Invalid index: " << index << " (valid range: 0-" 
                  << (splash.getImageCount() - 1) << ")\n";
        return 1;
    }
    
    std::cout << "Extracting image " << index << " to " << output_file << "...\n";
    if (!splash.extractImage(index, output_file)) {
        std::cerr << "Failed to extract image\n";
        return 1;
    }
    
    std::cout << "Extraction complete!\n";
    return 0;
}

int cmdReplace(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Error: replace requires <splash.img> <index> <input.png> <output.img>\n";
        return 1;
    }
    
    std::string input_splash = argv[2];
    int index = std::stoi(argv[3]);
    std::string input_png = argv[4];
    std::string output_splash = argv[5];
    
    bootmod::splash::SplashImage splash;
    if (!splash.load(input_splash)) {
        std::cerr << "Failed to load splash.img\n";
        return 1;
    }
    
    if (index < 0 || index >= static_cast<int>(splash.getImageCount())) {
        std::cerr << "Invalid index: " << index << " (valid range: 0-" 
                  << (splash.getImageCount() - 1) << ")\n";
        return 1;
    }
    
    std::cout << "Replacing image " << index << " with " << input_png << "...\n";
    if (!splash.replaceImage(index, input_png)) {
        std::cerr << "Failed to replace image\n";
        return 1;
    }
    
    std::cout << "Saving to " << output_splash << "...\n";
    if (!splash.save(output_splash)) {
        std::cerr << "Failed to save splash.img\n";
        return 1;
    }
    
    std::cout << "Replacement complete!\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    try {
        if (command == "unpack") {
            return cmdUnpack(argc, argv);
        } else if (command == "repack") {
            return cmdRepack(argc, argv);
        } else if (command == "info") {
            return cmdInfo(argc, argv);
        } else if (command == "extract") {
            return cmdExtract(argc, argv);
        } else if (command == "replace") {
            return cmdReplace(argc, argv);
        } else if (command == "version" || command == "--version" || command == "-v") {
            std::cout << "BootMod v" << VERSION << "\n";
            std::cout << "Universal Boot Logo/Splash Editor\n";
            std::cout << "Build: " << __DATE__ << " " << __TIME__ << "\n";
            std::cout << "License: MIT\n";
            std::cout << "Homepage: https://github.com/Badmaneers/BootMod\n";
            return 0;
        } else if (command == "help" || command == "--help" || command == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << "\n\n";
            printUsage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
