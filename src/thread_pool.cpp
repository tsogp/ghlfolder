#include "thread_pool.hpp"
#include <deque>
#include <future>
#include <mutex>

thread_pool::thread_pool(uint32_t threads_num) {
    const uint32_t current_threads_num = std::min(threads_num, std::thread::hardware_concurrency());
    for (int i = 0; i < current_threads_num; i++) {
        pool_.emplace_back(&thread_pool::run, this);
    }
}

thread_pool::~thread_pool() {
    is_active_ = false;
    cv_.notify_all();
    for (auto &th : pool_) {
        th.join();
    }
}

void thread_pool::push_job(std::packaged_task<void()> job) {
    std::scoped_lock lock(mutex_);
    pending_jobs_.emplace_back(std::move(job));
    cv_.notify_one();
}

void thread_pool::run() noexcept {
    while (is_active_) {
        thread_local std::packaged_task<void()> job;
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [&] { return !pending_jobs_.empty() || !is_active_; });
            if (!is_active_) {
                break;
            }
            job.swap(pending_jobs_.front());
            pending_jobs_.pop_front();
        }
        job();
    }
};