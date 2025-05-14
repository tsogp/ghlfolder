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
        total_bars_ = bars_.size();
        std::for_each(bars_.begin(), bars_.end(), [](const auto &bar) { bar->display(); });
    }

    std::size_t push_back(std::unique_ptr<fetch_bar> bar) {
        std::scoped_lock lck(mutex_);
        bar->display();
        bars_.push_back(std::move(bar));
        return total_bars_++;
    }

    void tick_i(std::size_t index, double progress) {
        std::scoped_lock lck(mutex_);
        std::size_t offset = total_bars_ - 1 - index;
        move_cursor_up(offset);
        bars_[index]->tick(progress);
        move_cursor_down(offset);
    }

    bool is_i_complete(std::size_t index) {
        std::scoped_lock lck(mutex_);
        return bars_[index]->is_complete();
    }

private:
    std::mutex mutex_;
    std::vector<std::unique_ptr<fetch_bar>> bars_;
    std::size_t total_bars_{};
    bool started_{};

    static void move_cursor_up(std::size_t l) {
        if (l != 0) {
            // TODO: think of a way to remove this offset
            std::cout << "\033[" << 1 << "C";
            std::cout << "\033[" << l << "A";
        }
    }

    static void move_cursor_down(std::size_t l) {
        if (l != 0) {
            // TODO: think of a way to remove this offset
            std::cout << "\033[" << 1 << "C";
            std::cout << "\033[" << l << "B";
        }
    }
};

#endif // __BAR_POOL_HPP__