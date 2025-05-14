#ifndef __CURL_WRAPPER_HPP__
#define __CURL_WRAPPER_HPP__

#include "fetch_bar.hpp"
#include <chrono>
#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;

class curl_wrapper {
public:
    explicit curl_wrapper(std::string url) : curl(curl_easy_init()), url(std::move(url)) {
        curl_global_init(CURL_GLOBAL_ALL);
        fetch_directory(this->url);
    }

    ~curl_wrapper() {
        if (curl != nullptr) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    void fetch_directory(const std::string &url, const std::string &base_path = ".") {
        std::string response;
        if (!http_get(url, response)) {
            std::cerr << "Failed to fetch directory listing: " << url << '\n';
            return;
        }

        json content = json::parse(response);
        for (const auto &entry : content) {
            std::string type = entry["type"];
            std::string path = entry["path"];
            std::string full_path = base_path + '/' + path;

            if (type == "file" && entry.contains("download_url")) {
                file_queue.emplace(entry["path"], entry["download_url"], entry["size"]);
            } else if (type == "dir") {
                std::filesystem::create_directories(full_path);
                fetch_directory(entry["url"]);
            }
        }
    }

    void download_files() {
        while (!file_queue.empty()) {
            auto [name, url, size] = file_queue.front();
            file_queue.pop();

            fetch_bar bar(name, size);
            bar.display();

            if (!download_file(name, url, bar)) {
                std::cerr << "Failed to download: " << name << '\n';
            }
        }
    }

private:
    CURL *curl;
    std::string url;
    std::queue<std::tuple<std::string, std::string, unsigned int>> file_queue;

    static size_t write_data(void *ptr, size_t size, size_t nmemb, std::ofstream *stream) {
        stream->write(static_cast<const char *>(ptr), size * nmemb);
        return size * nmemb;
    }

    bool http_get(const std::string &url, std::string &response) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl,
                         CURLOPT_USERAGENT,
                         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
                         "Chrome/94.0.4606.81 Safari/537.36");

        CURLcode res = curl_easy_perform(curl);
        return res == CURLE_OK;
    }

    static size_t write_callback(void *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append(static_cast<char *>(ptr), size * nmemb);
        return size * nmemb;
    }

    static size_t progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
        if (dltotal > 0) {
            auto *bar = static_cast<fetch_bar *>(clientp);
            double ratio = dlnow / dltotal;
            bar->tick(ratio);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return 0;
    }

    bool download_file(const std::string &filename, const std::string &url, fetch_bar &bar) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &bar);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

        CURLcode res = curl_easy_perform(curl);
        file.close();
        return res == CURLE_OK;
    }
};

#endif // __CURL_WRAPPER_HPP__
