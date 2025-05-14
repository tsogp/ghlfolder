#ifndef __REPO_HPP__
#define __REPO_HPP__

#include "bar_pool.hpp"
#include "fetch_bar.hpp"
#include "thread_pool.hpp"
#include <cpr/api.h>
#include <cpr/cpr.h>
#include <cstddef>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct r_data {
    std::string_view author;
    std::string_view name;
    std::string_view branch;
    std::string_view folder;
};

class r_base {
public:
    std::vector<std::future<void>> futures_;
    thread_pool worker_pool_;
    bar_pool bar_pool_;
    std::string url_;
    std::string_view output_dir_;
    r_data data;

    r_base() = delete;
    r_base(std::string_view author,
           std::string_view name,
           std::string_view branch,
           std::string_view folder,
           std::string_view output_dir)
        : data(author, name, branch, folder), worker_pool_(4), output_dir_(output_dir) {
        std::filesystem::create_directory(folder);
    }

    r_base(const r_base &br) = default;
    r_base(r_base &&br) = default;
    r_base &operator=(const r_base &br) = default;
    r_base &operator=(r_base &&br) = default;

    virtual void start() = 0;
    virtual ~r_base() = default;

    void wait_for_all() {
        for (auto &f : futures_) {
            f.wait();
        }
    }
};

class r_github : public r_base {
public:
    r_github(std::string_view author,
             std::string_view name,
             std::string_view branch,
             std::string_view folder,
             std::string_view output_dir)
        : r_base(author, name, branch, folder, output_dir) {
        url_ = std::format("https://api.github.com/repos/{}/{}/contents/{}?ref={}", author, name, folder, branch);
    }

    void handle_metadata_request(std::string url) {
        cpr::Response r = cpr::Get(cpr::Url{std::move(url)});
        if (r.status_code >= 400) {
            std::cerr << "Error [" << r.status_code << "] making request" << std::endl;
        } else {
            if (r.text.empty()) {
                return;
            }

            json data = json::parse(r.text);
            for (auto &it : data) {
                if (it["type"] == "file") {
                    auto task = std::packaged_task<void()>(
                        [name = it["path"], url = it["download_url"], file_size = it["size"], this] {
                            handle_request(name, url, file_size);
                        });
                    auto fut = task.get_future();
                    futures_.emplace_back(std::move(fut));
                    worker_pool_.push_job(std::move(task));
                } else if (it["type"] == "dir") {
                    fs::create_directory(it["path"]);
                    handle_metadata_request(std::move(it["url"]));
                }
            }
        }
    }

    void handle_request(const std::string &name, std::string url, unsigned int file_size) {
        std::ofstream of(name, std::ios::binary);
        const std::size_t idx = bar_pool_.push_back(std::make_unique<fetch_bar>(name, file_size));
        unsigned int downloaded = 0;

        cpr::Response r =
            cpr::Download(of,
                          cpr::Url{std::move(url)},
                          cpr::ProgressCallback([&](cpr::cpr_off_t downloadTotal,
                                                    cpr::cpr_off_t downloadNow,
                                                    cpr::cpr_off_t uploadTotal,
                                                    cpr::cpr_off_t uploadNow,
                                                    intptr_t userdata) -> bool {
                              bar_pool_.tick_i(idx, (static_cast<double>(downloadNow) - downloaded) / file_size);
                              downloaded = downloadNow;
                              return true;
                          }));

        // by here we know that downloading is completed
        if (downloaded < file_size) {
            bar_pool_.tick_i(idx, (static_cast<double>(file_size) - downloaded) / file_size);
        }
    }

    void start() override {
        if (fs::create_directory(data.folder)) {
            std::cerr << std::format("Error: couldn't create directory %s", data.folder);
            return;
        }

        // No endline character here because each new fetch bar starts with one
        std::cout << std::format("Cloning into {}...", data.folder);

        handle_metadata_request(std::move(url_));
        // std::cout << "Fetching repo data...\n";
        // cpr::Response r = cpr::Get(cpr::Url{std::move(url_)});
        // if (r.status_code >= 400) {
        //     std::cerr << "Error [" << r.status_code << "] making request" << std::endl;
        // } else {
        //     std::cout << "Request took " << r.text << std::endl;
        // json data = json::parse(r.text);
        // std::cout << data << '\n';
        // for (auto &it : data) {
        //     std::packaged_task<void()> task(
        //         [name = it["path"], url = it["download_url"], file_size = it["size"], this] {
        //             handle_request(name, url, file_size);
        //         });
        //     worker_pool_.push_job(std::move(task));
        // }
    }
};

class r_gitlab : public r_base {
public:
    r_gitlab(std::string_view author,
             std::string_view name,
             std::string_view branch,
             std::string_view folder,
             std::string_view output_dir)
        : r_base(author, name, branch, folder, output_dir) {
        url_ = std::format("https://gitlab.com/api/v4/projects/{}%2F{}", author, name);
    }

    void start() override {
    }
};

#endif // __REPO_HPP__