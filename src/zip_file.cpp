#include "zip_file.hpp"
#include "zip.h"
#include <format>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

zip_file::zip_file(const std::string& file_name) {
    int err = 0;
    archive = zip_open(file_name.c_str(), 0, &err);
    if (archive == nullptr) {
        throw std::runtime_error(
            std::format("Failed to open zip: {}", zip_error_strerror(zip_get_error(archive))));
    }
}

void zip_file::remove_unnecessary_dirs_and_save(std::string_view folder_to_keep) {
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(archive, i, 0);
        if (name == nullptr) {
            continue;
        }

        std::string_view entry_name(name);
        size_t outer_folder_end_idx = entry_name.find_first_of('/') + 1;
        entry_name.remove_prefix(outer_folder_end_idx);

        if (!entry_name.starts_with(folder_to_keep)) {
            if (zip_delete(archive, i) < 0) {
                throw std::runtime_error(std::format("Failed to delete {}\n", name));
            }
        } else {
            entry_name.remove_prefix(folder_to_keep.size());
            if (entry_name.ends_with('/')) {
                if (entry_name.size() != 1) {
                    entry_name.remove_prefix(1);
                }

                fs::create_directories(entry_name);
            } else {
                zip_file_t* zip_file = zip_fopen_index(archive, i, 0);
                if (zip_file == nullptr) {
                    throw std::runtime_error(std::format("Failed to open file in zip: {}\n", entry_name));
                }
                
                entry_name.remove_prefix(1);
                std::string entry_name_str = std::string(entry_name);
                std::ofstream out(entry_name_str, std::ios::binary);
                if (!out) {
                    zip_fclose(zip_file);
                    throw std::runtime_error(std::format("\nFailed to create output file: {}\n", entry_name_str));
                }

                zip_int64_t num_read = 0;
                while ((num_read = zip_fread(zip_file, buffer.data(), buffer_size)) > 0) {
                    out.write(buffer.data(), num_read);
                }

                zip_fclose(zip_file);
                out.close();
            }
        }
    }
}

zip_file::~zip_file() {
    if (archive != nullptr) {
        zip_discard(archive);
    }
}