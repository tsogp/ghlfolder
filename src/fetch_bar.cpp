#include "fetch_bar.hpp"
#include "term.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include "utils.hpp"

namespace {
    std::string truncate(const std::string &str, int max_length) {
        if (static_cast<int>(str.size()) > max_length) {
            return str.substr(0, max_length - 3) + "...";
        }
        return str;
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
        std::clamp(term_width - name_width_ - FETCH_BAR_SIZE_WIDTH - 5, BAR_MIN_WIDTH, BAR_MAX_WIDTH);
    spacer_width_ = std::max(term_width - (name_width_ + FETCH_BAR_SIZE_WIDTH + progress_width_), 5);
}

void fetch_bar::display(bool init) {
    std::cout << "\r";
    if (init) {
        std::cout << "\n";
    }
    std::cout << std::left << std::setw(name_width_) << truncate(name_, name_width_);
    std::cout << std::setw(spacer_width_) << "";
    std::cout << std::right << std::setw(FETCH_BAR_SIZE_WIDTH) << utils::print_size(file_size);
    if (init) {
        progress_.init_bar();
    } else {
        progress_.draw();
    }
}

void fetch_bar::tick(double step) {
    progress_.tick(step);
}


bool fetch_bar::is_complete() const {
    return progress_.is_complete();
}

void fetch_bar::set_row_idx(unsigned int row_idx) {
    this->row_idx = row_idx;
}

unsigned int fetch_bar::get_row_idx() const {
    return row_idx;
}
