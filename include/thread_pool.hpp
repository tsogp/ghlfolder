#include "fetch_bar.hpp"
#include <condition_variable>
#include <curl/curl.h>
#include <curl_wrapper.hpp>
#include <functional>
#include <nlohmann/json.hpp>
#include <queue>
#include <sys/stat.h>
#include <thread>
#include <vector>

class thread_pool {
public:
    thread_pool(const thread_pool &tp) = delete;
    thread_pool(thread_pool &&tp) = delete;
    thread_pool &operator=(const thread_pool &tp) = delete;
    thread_pool &operator=(thread_pool &&tp) = delete;

    explicit thread_pool(size_t thread_count) {
        for (size_t i = 0; i < thread_count; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return !tasks.empty() || stop; });
                        if (stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    ~thread_pool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};
