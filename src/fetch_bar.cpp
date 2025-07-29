#include "fetch_bar.hpp"
#include "term.hpp"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
    std::string truncate(const std::string &str, int max_length) {
        if (static_cast<int>(str.size()) > max_length) {
            return str.substr(0, max_length - 3) + "...";
        }
        return str;
    }

    std::string print_size(unsigned int bytes) {
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
}

fetch_bar::fetch_bar(std::string name, unsigned int file_size)
    : name_(std::move(name)), progress_(BAR_MIN_WIDTH), file_size(file_size) {
        update_layout();
        progress_.resize(progress_width_);
    }

void fetch_bar::update_layout() {
    int term_width = term_data::get_width();

    name_width_ = std::clamp(static_cast<int>(name_.size()), 5, 70);
    progress_width_ =
        std::clamp(term_width - name_width_ - _SIZE_WIDTH - 5, BAR_MIN_WIDTH, BAR_MAX_WIDTH);// + BAR_POSTFIX_SIZE + 1;
    spacer_width_ = std::max(term_width - (name_width_ + _SIZE_WIDTH + progress_width_), 5);
}

void fetch_bar::display() {
    std::cout << "\r\n" << std::left << std::setw(name_width_) << truncate(name_, name_width_);
    std::cout << std::setw(spacer_width_) << "";
    std::cout << std::right << std::setw(_SIZE_WIDTH) << print_size(file_size);
    progress_.init_bar();
}

void fetch_bar::tick(double step) {
    progress_.tick(step);
}

bool fetch_bar::is_complete() const {
    return progress_.is_complete();
}
