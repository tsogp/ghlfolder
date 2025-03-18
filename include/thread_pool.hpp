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

    void download_directory(const std::string &url, const std::string &local_path) {
        enqueue([this, url, local_path] { process_directory(url, local_path); });
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

    void process_directory(const std::string &url, const std::string &local_path) {
        nlohmann::json response = curl_wrapper::fetch(url);

        std::filesystem::create_directories(local_path);
        std::filesystem::permissions(local_path,
                                     std::filesystem::perms::others_all,
                                     std::filesystem::perm_options::remove);

        for (const auto &entry : response) {
            std::string name = entry["path"];
            std::string path = local_path + "/" + name;

            if (entry["type"] == "file" && !entry["download_url"].is_null()) {
                std::string download_url = entry["download_url"];
                unsigned int file_size = entry["size"];

                enqueue([download_url, path, file_size] {
                    fetch_bar bar(path, file_size);
                    bar.display();
                    curl_wrapper::download_file(download_url, path, bar);
                });

            } else if (entry["type"] == "dir") {
                std::string new_url = entry["url"];
                download_directory(new_url, path);
            }
        }
    }
};
