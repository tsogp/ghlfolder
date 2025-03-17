#ifndef __PROGRESS_BAR_HPP__
#define __PROGRESS_BAR_HPP__

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

static constexpr int BAR_POSTFIX_SIZE = 7;
static constexpr int BAR_MIN_WIDTH = 10;
static constexpr int BAR_MAX_WIDTH = 100;

template <int MIN_WIDTH = BAR_MIN_WIDTH, int MAX_WIDTH = BAR_MAX_WIDTH>
class progress_bar {
public:
    progress_bar() = delete;

    explicit progress_bar(int width) : width_(std::clamp(width, MIN_WIDTH, MAX_WIDTH)) {
    }

    void init_bar() {
        if (!is_initiated) {
            std::cout << "[" << std::setw(pos_) << std::setfill('#') << "";
            std::cout << std::setw(width_ - pos_) << std::setfill(' ') << "";
            std::cout << ']' << std::right << std::setw(5) << std::setfill(' ') << static_cast<int>(ceil(progress_ * 100.0)) << "%";
            is_initiated = true;
        }
    }

    void tick(double step) {
        init_bar();

        if (progress_ >= 1.0) {
            return;
        }

        progress_ = std::min(progress_ + step, 1.0);
        int new_pos = static_cast<int>(ceil(width_ * progress_));
        int char_progress = new_pos - pos_;

        std::cout << "\033[" << width_ - pos_ - 1 + BAR_POSTFIX_SIZE << "D";
        std::cout << std::setw(char_progress) << std::setfill('#') << "";
        std::cout << "\033[" << (width_ - new_pos + 1) << "C";
        std::cout << std::setw(5) << std::setfill(' ') << static_cast<int>(ceil(progress_ * 100.0)) << "%";
        std::cout.flush();

        pos_ = new_pos;
    }

    void resize(int new_width) {
        is_initiated = false;
        width_ = std::clamp(new_width, MIN_WIDTH, MAX_WIDTH);
        pos_ = static_cast<int>(ceil(width_ * progress_));
        init_bar();
    }

    [[nodiscard]] bool is_complete() const {
        return progress_ >= 1.0;
    }

private:
    int width_, pos_{};
    double progress_{};
    bool is_initiated{};
};


#endif // __PROGRESS_BAR_HPP__
