#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <condition_variable>
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

    void push_job(std::packaged_task<bool(std::stop_token)> job);
    bool stop_all();
    void wait_for_all();
    size_t jobs_in_progress_ = 0;
    std::deque<std::packaged_task<bool(std::stop_token)>> pending_jobs_;
private:
    std::vector<std::jthread> pool_;
    std::atomic_bool is_active_{true};
    std::condition_variable cv_, out_of_work_;
    std::mutex mutex_;


    void run(std::stop_token stoken) noexcept;
};

#endif // __THREAD_POOL_HPP__