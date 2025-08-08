#include "utils.hpp"
#include "term.hpp"
#include <array>
#include <sstream>
#include <iomanip>

void utils::global_cleanup() {
    term_data::show_cursor();
}

std::string utils::print_size(unsigned int bytes) {
    static constexpr std::array<std::string_view, 4> units = {"B", "KB", "MB", "GB"}; 
    double size = bytes;
    int unitIndex = 0;

    while (size >= 1024 && unitIndex < 3) {
        size /= 1024;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex] << " ";

    return oss.str();
}