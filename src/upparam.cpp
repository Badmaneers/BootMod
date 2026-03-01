#include "upparam.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace samsung {

#pragma pack(push, 1)
struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};
#pragma pack(pop)

static void computeChecksum(TarHeader& header) {
    memset(header.chksum, ' ', 8);
    unsigned int sum = 0;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&header);
    for (size_t i = 0; i < 512; ++i) {
        sum += p[i];
    }
    snprintf(header.chksum, 8, "%06o", sum);
}

bool UpParam::unpack(const std::string& up_param_file, const std::string& output_dir) {
    std::ifstream file(up_param_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << up_param_file << " for reading.\n";
        return false;
    }

    fs::create_directories(output_dir);
    std::cout << "Unpacking Samsung up_param (" << up_param_file << ")...\n";

    TarHeader header;
    while (file.read(reinterpret_cast<char*>(&header), sizeof(TarHeader))) {
        if (header.name[0] == '\0') {
            break; // End of archive
        }

        std::string filename(header.name);
        
        // Parse size (octal)
        size_t size = 0;
        try {
            size = std::stoull(header.size, nullptr, 8);
        } catch(...) {
            // Might be an issue, skip or fail
            return false;
        }

        // Only process normal files (typeflag '0' or '\0')
        if (header.typeflag[0] == '0' || header.typeflag[0] == '\0') {
            std::string out_path = output_dir + "/" + fs::path(filename).filename().string();
            std::ofstream out(out_path, std::ios::binary);
            if (!out.is_open()) {
                std::cerr << "Failed to create output file " << out_path << "\n";
                return false;
            }

            std::cout << "Extracting: " << filename << " (" << size << " bytes)\n";

            std::vector<char> buffer(4096);
            size_t remaining = size;
            while (remaining > 0) {
                size_t to_read = std::min(remaining, buffer.size());
                file.read(buffer.data(), to_read);
                out.write(buffer.data(), to_read);
                remaining -= to_read;
            }
        } else {
            // Skip data of unsupported types
            file.seekg(size, std::ios::cur);
        }

        // Skip padding to next 512 byte boundary
        size_t padding = (512 - (size % 512)) % 512;
        if (padding > 0) {
            file.seekg(padding, std::ios::cur);
        }
    }

    std::cout << "Extraction complete!\n";
    return true;
}

std::vector<UpParamEntry> UpParam::read(const std::string& up_param_file) {
    std::vector<UpParamEntry> entries;
    std::ifstream file(up_param_file, std::ios::binary);
    if (!file.is_open()) {
        return entries;
    }

    TarHeader header;
    while (file.read(reinterpret_cast<char*>(&header), sizeof(TarHeader))) {
        if (header.name[0] == '\0') {
            break; // End of archive
        }

        std::string filename(header.name);
        
        size_t size = 0;
        try {
            size = std::stoull(header.size, nullptr, 8);
        } catch(...) {
            break;
        }

        if (header.typeflag[0] == '0' || header.typeflag[0] == '\0') {
            UpParamEntry entry;
            entry.filename = filename;
            entry.data.resize(size);
            
            if (size > 0) {
                file.read(reinterpret_cast<char*>(entry.data.data()), size);
            }
            entries.push_back(std::move(entry));
        } else {
            file.seekg(size, std::ios::cur);
        }

        size_t padding = (512 - (size % 512)) % 512;
        if (padding > 0) {
            file.seekg(padding, std::ios::cur);
        }
    }

    return entries;
}

bool UpParam::repack(const std::vector<std::string>& input_files, const std::string& output_file) {
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << output_file << " for writing.\n";
        return false;
    }

    std::cout << "Repacking to " << output_file << "...\n";

    for (const auto& file_path : input_files) {
        std::ifstream in(file_path, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Failed to open input file: " << file_path << "\n";
            continue;
        }

        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        in.seekg(0, std::ios::beg);

        TarHeader header = {};
        
        // Use just the filename without paths
        std::string filename = fs::path(file_path).filename().string();
        strncpy(header.name, filename.c_str(), sizeof(header.name) - 1);
        
        snprintf(header.mode, sizeof(header.mode), "%07o", 0644);
        snprintf(header.uid, sizeof(header.uid), "%07o", 0);
        snprintf(header.gid, sizeof(header.gid), "%07o", 0);
        snprintf(header.size, sizeof(header.size), "%011zo", size);
        snprintf(header.mtime, sizeof(header.mtime), "%011zo", (size_t)time(nullptr));
        header.typeflag[0] = '0';
        std::memcpy(header.magic, "ustar ", 6);
        header.version[0] = ' ';
        header.version[1] = '\0';
        strncpy(header.uname, "root", sizeof(header.uname) - 1);
        strncpy(header.gname, "root", sizeof(header.gname) - 1);

        computeChecksum(header);

        out.write(reinterpret_cast<char*>(&header), sizeof(TarHeader));

        std::cout << "Adding: " << filename << " (" << size << " bytes)\n";

        std::vector<char> buffer(4096);
        while (in.read(buffer.data(), buffer.size())) {
            out.write(buffer.data(), in.gcount());
        }
        if (in.gcount() > 0) {
            out.write(buffer.data(), in.gcount());
        }

        // Add padding
        size_t padding = (512 - (size % 512)) % 512;
        if (padding > 0) {
            std::vector<char> pad(padding, 0);
            out.write(pad.data(), padding);
        }
    }

    // Write end of archive (two 512 byte blocks of zeros)
    std::vector<char> zeros(1024, 0);
    out.write(zeros.data(), 1024);

    std::cout << "Repack complete!\n";
    return true;
}

} // namespace samsung
