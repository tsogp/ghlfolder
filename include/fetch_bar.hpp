#ifndef __FETCH_BAR_HPP__
#define __FETCH_BAR_HPP__

#include "progress_bar.hpp"
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

class fetch_bar {
public:
    explicit fetch_bar(std::string name, unsigned int file_size);

    fetch_bar() = default;
    fetch_bar(fetch_bar &&b) = default;
    fetch_bar(const fetch_bar &b) = default;
    fetch_bar &operator=(fetch_bar &&b) = default;
    fetch_bar &operator=(const fetch_bar &b) = default;

    void update_layout();
    void display();
    void tick(double step);
    [[nodiscard]] bool is_complete() const;

private:
    std::string name_;
    int name_width_ = 5;
    int spacer_width_ = 5;
    int progress_width_ = 10;
    progress_bar<> progress_;
    
    unsigned int file_size;

    static constexpr int _SIZE_WIDTH = 10;
};

#endif // __FETCH_BAR_HPP__
