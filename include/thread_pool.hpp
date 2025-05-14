#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <cstdint>
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

// Implementation heavily influenced by https://nixiz.github.io/yazilim-notlari/2023/10/07/thread_pool-en

class thread_pool {
public:
    explicit thread_pool(uint32_t threads_num = 1) {
        const uint32_t current_threads_num = std::min(threads_num, std::thread::hardware_concurrency());
        for (int i = 0; i < current_threads_num; i++) {
            pool_.emplace_back(&thread_pool::run, this);
        }
    }

    ~thread_pool() {
        is_active_ = false;
        cv_.notify_all();
        for (auto &th : pool_) {
            th.join();
        }
    }

    void push_job(std::packaged_task<void()> job) {
        std::scoped_lock lock(mutex_);
        pending_jobs_.emplace_back(std::move(job));
        cv_.notify_one();
    }

private:
    std::vector<std::thread> pool_;
    std::atomic_bool is_active_{true};
    std::deque<std::packaged_task<void()>> pending_jobs_;
    std::condition_variable cv_;
    std::mutex mutex_;

    void run() noexcept {
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
};

#endif // __THREAD_POOL_HPP__