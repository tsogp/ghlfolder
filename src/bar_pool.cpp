#include "bar_pool.hpp"
#include "fetch_bar.hpp"
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace {
    void move_cursor_up(std::size_t l) {
        if (l != 0) {
            // TODO: think of a way to remove this offset
            std::cout << "\033[" << 1 << "C";
            std::cout << "\033[" << l << "A";
        }
    }

    void move_cursor_down(std::size_t l) {
        if (l != 0) {
            // TODO: think of a way to remove this offset
            std::cout << "\033[" << 1 << "C";
            std::cout << "\033[" << l << "B";
        }
    }
};

template bar_pool::bar_pool();

template <typename... Bars>
bar_pool::bar_pool(Bars &&...bars) {
    (bars_.emplace_back(std::move(bars)), ...);
    total_bars_ = bars_.size();
    std::for_each(bars_.begin(), bars_.end(), [](const auto &bar) { bar->display(); });
}

std::size_t bar_pool::push_back(std::unique_ptr<fetch_bar> bar) {
    std::scoped_lock lck(mutex_);
    bar->display();
    bars_.push_back(std::move(bar));
    return total_bars_++;
}

void bar_pool::tick_i(std::size_t index, double progress) {
    std::scoped_lock lck(mutex_);
    std::size_t offset = total_bars_ - 1 - index;
    move_cursor_up(offset);
    bars_[index]->tick(progress);
    move_cursor_down(offset);
}

bool bar_pool::is_i_complete(std::size_t index) {
    std::scoped_lock lck(mutex_);
    return bars_[index]->is_complete();
}