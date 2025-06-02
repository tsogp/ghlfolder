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
    explicit thread_pool(uint32_t threads_num = 1);
    ~thread_pool();

    void push_job(std::packaged_task<void()> job);
private:
    std::vector<std::thread> pool_;
    std::atomic_bool is_active_{true};
    std::deque<std::packaged_task<void()>> pending_jobs_;
    std::condition_variable cv_;
    std::mutex mutex_;

    void run() noexcept;
};

#endif // __THREAD_POOL_HPP__