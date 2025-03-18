#ifndef __FETCH_BAR_HPP__
#define __FETCH_BAR_HPP__

#include "progress_bar.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

class fetch_bar {
public:
    explicit fetch_bar(std::string name, unsigned int file_size)
        : name_(std::move(name)), progress_(get_terminal_width()), file_size(file_size) {
        update_layout();
    }

    void update_layout() {
        int term_width = get_terminal_width();

        name_width_ = std::min(std::max(static_cast<int>(name_.size()), 5), 70);
        progress_width_ =
            std::clamp(term_width - name_width_ - SIZE_WIDTH - 5, BAR_MIN_WIDTH, BAR_MAX_WIDTH) + BAR_POSTFIX_SIZE + 1;
        spacer_width_ = std::max(term_width - (name_width_ + SIZE_WIDTH + progress_width_), 5);
    }

    void display() {
        std::cout << std::setw(name_width_) << truncate(name_, name_width_) << "";
        std::cout << std::left << std::setw(spacer_width_) << "";
        std::cout << std::right << std::setw(SIZE_WIDTH) << print_size(file_size) << "";
        progress_.init_bar();
    }

    void tick(double step) {
        progress_.tick(step);
    }

    [[nodiscard]] bool is_complete() const {
        return progress_.is_complete();
    }

private:
    std::string name_;
    progress_bar<> progress_;
    int name_width_ = 5;
    int spacer_width_ = 5;
    int progress_width_ = 10;

    unsigned int file_size;

    static constexpr int SIZE_WIDTH = 10;

    static std::string truncate(const std::string &str, int max_length) {
        if (static_cast<int>(str.size()) > max_length) {
            return str.substr(0, max_length - 3) + "...";
        }
        return str;
    }

    static int get_terminal_width() {
        winsize size{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        return size.ws_col;
    }

    static std::string print_size(unsigned int bytes) {
        const char *units[] = {"B", "KB", "MB", "GB"};
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
};

#endif // __FETCH_BAR_HPP__
