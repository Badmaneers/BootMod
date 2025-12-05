#include "bootmod.h"
#include <iostream>
#include <cstring>
#include <vector>

#ifndef VERSION
#define VERSION "1.6.0"
#endif

void printUsage(const char* program) {
    std::cout << "BootMod - Universal Boot Logo/Splash Editor v" << VERSION << "\n";
    std::cout << "Supports: MediaTek logo.bin, Qualcomm splash.img\n\n";
    std::cout << "Usage: " << program << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  unpack <logo.bin> <output_dir> [options]\n";
    std::cout << "    Extract logos from logo.bin file\n";
    std::cout << "    Options:\n";
    std::cout << "      --mode <mode>        Color mode (bgrabe, bgrale, rgbabe, rgbale, rgb565be, rgb565le)\n";
    std::cout << "      --slots <0,1,2>      Extract only specific slots (comma-separated)\n";
    std::cout << "      --raw                Extract as raw .z files without decompression\n";
    std::cout << "      --flip               Flip orientation\n";
    std::cout << "\n";
    std::cout << "  repack <output.bin> <file1> <file2> ... [options]\n";
    std::cout << "    Repack logo files into logo.bin\n";
    std::cout << "    Options:\n";
    std::cout << "      --strip-alpha        Remove alpha channel\n";
    std::cout << "\n";
    std::cout << "  info <logo.bin>\n";
    std::cout << "    Display information about logo.bin file\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program << " unpack logo.bin extracted/ --mode bgrabe\n";
    std::cout << "  " << program << " unpack logo.bin extracted/ --mode bgrabe --slots 0,1\n";
    std::cout << "  " << program << " repack new_logo.bin extracted/logo_*.png\n";
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
        std::cerr << "Error: unpack requires <logo.bin> <output_dir>\n";
        return 1;
    }
    
    std::string logo_file = argv[2];
    std::string output_dir = argv[3];
    
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
    
    return mtklogo::MtkLogo::unpack(logo_file, output_dir, mode, slots, extract_raw, flip) ? 0 : 1;
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
        std::cerr << "Error: info requires <logo.bin>\n";
        return 1;
    }
    
    try {
        std::string logo_file = argv[2];
        mtklogo::LogoImage image = mtklogo::LogoImage::readFromFile(logo_file);
        
        std::cout << "MTK Logo Information\n";
        std::cout << "====================\n";
        std::cout << "File: " << logo_file << "\n";
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
