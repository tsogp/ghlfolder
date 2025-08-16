#include "thread_pool.hpp"
#include <deque>
#include <future>
#include <mutex>

thread_pool::thread_pool(uint32_t threads_num) {
    const uint32_t current_threads_num = std::min(threads_num, std::thread::hardware_concurrency());
    for (uint32_t i = 0; i < current_threads_num; ++i) {
        pool_.emplace_back([this](std::stop_token stoken) { run(stoken); });
    }
}

thread_pool::~thread_pool() {
    is_active_ = false;
    cv_.notify_all();
    // std::jthread automatically joins in destructor
}

void thread_pool::push_job(std::packaged_task<bool(std::stop_token)> job) {
    std::scoped_lock lock(mutex_);
    if (!is_active_) {
        throw thread_pool_stopped{};
    }
    pending_jobs_.emplace_back(std::move(job));
    cv_.notify_one();
}

void thread_pool::run(std::stop_token stoken) noexcept {
    while (true) {
        std::packaged_task<bool(std::stop_token)> job;
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [&] {
                return !pending_jobs_.empty() || !is_active_ || stoken.stop_requested();
            });

            if (!is_active_ || stoken.stop_requested()) {
                break;
            }

            job.swap(pending_jobs_.front());
            pending_jobs_.pop_front();
            ++jobs_in_progress_;
        }

        job(stoken);

        {
            std::lock_guard lock(mutex_);
            --jobs_in_progress_;
            if (!is_active_ || jobs_in_progress_ == 0) {
                out_of_work_.notify_all();
            }
        }
    }
}

void thread_pool::wait_for_all() {
    std::unique_lock lock(mutex_);
    out_of_work_.wait(lock, [&] {
        return jobs_in_progress_ == 0;
    });
}
bool thread_pool::stop_all() {
    {
        std::unique_lock lock(mutex_);
        is_active_ = false;
        pending_jobs_.clear();
        cv_.notify_all();
    }
    wait_for_all();
    return true;
}
