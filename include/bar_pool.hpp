#ifndef __BAR_POOL_HPP__
#define __BAR_POOL_HPP__

#include "fetch_bar.hpp"
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
    explicit bar_pool(Bars &&...bars);
    std::size_t push_back(std::unique_ptr<fetch_bar> bar);
    void tick_i(std::size_t index, double progress);
    bool is_i_complete(std::size_t index);
private:
    std::mutex mutex_;
    std::vector<std::unique_ptr<fetch_bar>> bars_;
    std::size_t total_bars_{};
    bool started_{};
};

#endif // __BAR_POOL_HPP__