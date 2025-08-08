#include "bar_pool.hpp"
#include "fetch_bar.hpp"
#include "term.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

template bar_pool::bar_pool();

template <typename... Bars>
bar_pool::bar_pool(Bars &&...bars) {
    (bars_.emplace_back(std::move(bars)), ...);
    total_bars_ = bars_.size();
    size_t idx = 1;
    for (auto it = bars_.begin(); it != bars_.end(); ++it, ++idx) {
        (*it)->set_row_idx(idx);
        current_rows.insert((*it).get());
        (*it)->display();
    }
}

std::size_t bar_pool::push_back(std::unique_ptr<fetch_bar> bar) {
    std::scoped_lock lck(mutex_);
    // Set the row index to the bar
    bar->set_row_idx(total_bars_ + 1);
    current_rows.insert(bar.get());
    bar->display();
    bars_.push_back(std::move(bar));
    return total_bars_++;
}

void bar_pool::tick_i(std::size_t index, double progress) {
    std::scoped_lock lck(mutex_);
    std::size_t offset = total_bars_ - bars_[index]->get_row_idx();
    term_data::move_cursor_up(offset);
    bars_[index]->tick(progress);

    // Push the completed bar into bottom and non-completed to the top
    auto* oldest = *current_rows.begin();
    if (bars_[index]->is_complete() && bars_[index]->get_row_idx() > oldest->get_row_idx()) {
        term_data::move_cursor_down(offset);
        
        std::size_t oldest_active_bar_offset = total_bars_ - oldest->get_row_idx();
        term_data::move_cursor_up(oldest_active_bar_offset);
        bars_[index]->display(false);
        
        term_data::move_cursor_down(oldest_active_bar_offset);
        term_data::move_cursor_up(offset);
        oldest->display(false);

        current_rows.erase(bars_[index].get());
        current_rows.erase(oldest);

        oldest->set_row_idx(bars_[index]->get_row_idx());
        if (!oldest->is_complete()) {
            current_rows.insert(oldest);
        }
    }

    term_data::move_cursor_down(offset);
}

bool bar_pool::is_i_complete(std::size_t index) {
    std::scoped_lock lck(mutex_);
    return bars_[index]->is_complete();
}

bar_pool::~bar_pool() {
    for (const auto& bar: bars_) {
        if (!bar->is_complete()) {
            bar->tick(1);
        }
    }
}