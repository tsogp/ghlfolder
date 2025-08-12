#ifndef __ZIP_FILE_HPP__
#define __ZIP_FILE_HPP__

#include "zip.h"
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

class zip_file {
public:
    explicit zip_file(const std::string& file_name);
    ~zip_file();
    void remove_unnecessary_dirs_and_save(std::string_view folder_to_keep);

private:
    static constexpr size_t buffer_size = 1024;    
    std::array<char, buffer_size> buffer = {};
    zip_t* archive = nullptr;
};

#endif // __ZIP_FILE_HPP__