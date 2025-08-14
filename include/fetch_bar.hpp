#ifndef __FETCH_BAR_HPP__
#define __FETCH_BAR_HPP__

#include "progress_bar.hpp"
#include <string>

constexpr int FETCH_BAR_SIZE_WIDTH = 10;

class fetch_bar {
public:
    explicit fetch_bar(std::string name, unsigned int file_size);

    fetch_bar() = default;
    fetch_bar(fetch_bar &&b) = default;
    fetch_bar(const fetch_bar &b) = default;
    fetch_bar &operator=(fetch_bar &&b) = default;
    fetch_bar &operator=(const fetch_bar &b) = default;

    void update_layout();
    void set_row_idx(unsigned int row_idx);
    void display(bool init = true);
    void tick(double step);
    [[nodiscard]] unsigned int get_row_idx() const;
    [[nodiscard]] bool is_complete() const;

private:
    std::string name_;
    int name_width_ = 5;
    int spacer_width_ = 5;
    int progress_width_ = 10;
    unsigned int row_idx = 0;
    progress_bar<> progress_;
    
    unsigned int file_size;
};

#endif // __FETCH_BAR_HPP__
