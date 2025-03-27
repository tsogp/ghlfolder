#ifndef __BAR_POOL_HPP__
#define __BAR_POOL_HPP__

#include "fetch_bar.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

class bar_pool {
public:
    bar_pool(const bar_pool &bp) = delete;
    bar_pool(bar_pool &&bp) = delete;
    bar_pool &operator=(const bar_pool &bp) = delete;
    bar_pool &operator=(bar_pool &&bp) = delete;

    template <typename... Bars>
    explicit bar_pool(Bars &&...bars) {
        (bars_.emplace_back(std::move(bars)), ...);
        active_bar_idx_ = bars_.size() - 1;
        std::for_each(bars_.begin(), bars_.end(), [](const auto& bar) { bar->display(); });
    }

    void push_back(std::unique_ptr<fetch_bar> bar) {
        std::scoped_lock lck(mutex_);
        bar->display();
        bars_.push_back(std::move(bar));
    }

    fetch_bar &operator[](std::size_t index) {
        std::scoped_lock lck(mutex_);
        if (index > active_bar_idx_) {
            move_cursor_down(index - active_bar_idx_);
        } else if (index < active_bar_idx_) {
            move_cursor_up(active_bar_idx_ - index);
        }

        active_bar_idx_ = index;
        return *bars_[index];
    }

private:
    std::mutex mutex_;
    std::vector<std::unique_ptr<fetch_bar>> bars_;
    std::size_t active_bar_idx_{};
    bool started_{};

    static void move_cursor_up(std::size_t l) {
        // TODO: think of a way to remove this offset
        std::cout << "\033[" << 1 << "C";
        std::cout << "\033[" << l << "A";
    }

    static void move_cursor_down(std::size_t l) {
        // TODO: think of a way to remove this offset
        std::cout << "\033[" << 1 << "C";
        std::cout << "\033[" << l << "B";
    }
};

#endif // __BAR_POOL_HPP__