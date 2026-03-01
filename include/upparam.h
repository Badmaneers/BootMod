#ifndef UPPARAM_H
#define UPPARAM_H

#include <string>
#include <vector>
#include <cstdint>

namespace samsung {

struct UpParamEntry {
    std::string filename;
    std::vector<uint8_t> data;
};

class UpParam {
public:
    // Extract a Samsung up_param tar archive to output directory
    static bool unpack(const std::string& up_param_file, const std::string& output_dir);
    
    // Repack images to a Samsung up_param tar archive
    static bool repack(const std::vector<std::string>& input_files, const std::string& output_file);
    
    // Read up_param entries into memory
    static std::vector<UpParamEntry> read(const std::string& up_param_file);
};

} // namespace samsung

#endif // UPPARAM_H
