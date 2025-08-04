#ifndef __PROGRESS_BAR_HPP__
#define __PROGRESS_BAR_HPP__

#include <sys/ioctl.h>
#include <unistd.h>

static constexpr int BAR_POSTFIX_SIZE = 7;
static constexpr int BAR_MIN_WIDTH = 10;
static constexpr int BAR_MAX_WIDTH = 100;

template <int MIN_WIDTH = BAR_MIN_WIDTH, int MAX_WIDTH = BAR_MAX_WIDTH>
class progress_bar {
public:
    progress_bar() = delete;

    explicit progress_bar(int width);
    void init_bar();
    void tick(double step);
    void draw();
    void resize(int new_width, bool reinit = false);
    [[nodiscard]] bool is_complete() const;
private:
    int width_, pos_{};
    double progress_{};
    bool is_initiated{};
};

#endif // __PROGRESS_BAR_HPP__
