#include "progress_bar.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

template <int MIN_WIDTH, int MAX_WIDTH>
progress_bar<MIN_WIDTH, MAX_WIDTH>::progress_bar(int width) : width_(std::clamp(width, MIN_WIDTH, MAX_WIDTH) - (BAR_POSTFIX_SIZE + 1)) {}

template <int MIN_WIDTH, int MAX_WIDTH>
void progress_bar<MIN_WIDTH, MAX_WIDTH>::init_bar() {
    if (!is_initiated) {
        draw();
        is_initiated = true;
    }
}

template <int MIN_WIDTH, int MAX_WIDTH>
void progress_bar<MIN_WIDTH, MAX_WIDTH>::tick(double step) {
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

template <int MIN_WIDTH, int MAX_WIDTH>
void progress_bar<MIN_WIDTH, MAX_WIDTH>::resize(int new_width, bool reinit) {
    is_initiated = false;
    width_ = std::clamp(new_width, MIN_WIDTH, MAX_WIDTH) - (BAR_POSTFIX_SIZE + 1);
    pos_ = static_cast<int>(ceil(width_ * progress_));
    if (reinit) {
        init_bar();
    }
}

template <int MIN_WIDTH, int MAX_WIDTH>
void progress_bar<MIN_WIDTH, MAX_WIDTH>::draw() {
    std::cout << "[" << std::setw(pos_) << std::setfill('#') << "";
    std::cout << std::setw(width_ - pos_) << std::setfill(' ') << "";
    std::cout << ']' << std::right << std::setw(5) << std::setfill(' ') << static_cast<int>(ceil(progress_ * 100.0)) << "%";
}

template <int MIN_WIDTH, int MAX_WIDTH>
bool progress_bar<MIN_WIDTH, MAX_WIDTH>::is_complete() const {
    return progress_ >= 1.0;
}

template class progress_bar<BAR_MIN_WIDTH, BAR_MAX_WIDTH>;