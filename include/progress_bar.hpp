#ifndef __PROGRESS_BAR_HPP__
#define __PROGRESS_BAR_HPP__

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

template <int MIN_WIDTH = 10, int MAX_WIDTH = 100>
class progress_bar {
public:
    explicit progress_bar(int width, bool init = false) : width_(std::clamp(width, MIN_WIDTH, MAX_WIDTH)) {
        if (init) {
            init_bar();
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

        std::cout << "\033[" << width_ - pos_ + BAR_POSTFIX_SIZE << "D";
        std::cout << std::setw(char_progress) << std::setfill('#') << "";
        std::cout << "\033[" << (width_ - new_pos + 1) << "C";
        std::cout << std::setw(5) << std::setfill(' ') << static_cast<int>(ceil(progress_ * 100.0)) << "%";
        std::cout.flush();

        pos_ = new_pos;
    }

    [[nodiscard]] bool is_complete() const {
        return progress_ >= 1.0;
    }

private:
    void init_bar() {
        if (!is_initiated) {
            std::cout << "[" << std::setw(width_ + BAR_POSTFIX_SIZE) << std::setfill(' ') << "]    0%";
            is_initiated = true;
        }
    }

    static constexpr int BAR_POSTFIX_SIZE = 7;

    int width_, pos_{};
    double progress_{};
    bool is_initiated{};
};


#endif // __PROGRESS_BAR_HPP__
