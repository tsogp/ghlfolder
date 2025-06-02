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
#include <string_view>

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
    r_data data;

    r_base() = delete;
    r_base(std::string_view author,
           std::string_view name,
           std::string_view branch,
           std::string_view folder,
            bool create_dir = false)
        : data(author, name, branch, folder), worker_pool_(4) {
        if (create_dir) {
            if (fs::exists(folder)) {
                if (!fs::is_empty(folder)) {
                    std::cerr << std::format("Error: output directory '{}' already exists and it is not empty.\n", folder);
                    exit(1);
                }
            } else if (!fs::create_directory(folder)) {
                std::cerr << std::format("Error: couldn't create directory {}", folder);
                exit(1);
            }

            fs::current_path(folder);

            // No endline character here because each new fetch bar starts with one
            std::cout << std::format("Cloning into '{}'...", folder);
        };

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
            bool create_dir = false)
        : r_base(author, name, branch, preprocess_folder(folder), create_dir) {
        url_ = std::format("https://api.github.com/repos/{}/{}/contents/{}?ref={}", author, name, folder, branch);
    }

    void handle_metadata_request(std::string url) {
        cpr::Response r = cpr::Get(cpr::Url{std::move(url)});
        if (r.status_code == 0) {
            std::cerr << "\nError: " << r.error.message << '\n';
            exit(1);
        } else if (r.status_code == 404) {
            std::cerr << "\nServer returned 404: does not exist. Check again if the repository URL is correct.";
            std::exit(1);
        } else {
            if (r.text.empty()) {
                return;
            }

            json data = json::parse(r.text);
            for (auto &it : data) {
                // TODO: fix to avoid copying
                std::string trimmed_path = it["path"].template get<std::string>().substr(pathb_idx_);
                if (it["type"] == "file") {
                    auto task = std::packaged_task<void()>(
                        [name = trimmed_path, url = it["download_url"], file_size = it["size"], this] {
                            handle_request(name, url, file_size);
                        });
                    auto fut = task.get_future();
                    futures_.emplace_back(std::move(fut));
                    worker_pool_.push_job(std::move(task));
                } else if (it["type"] == "dir") {
                    fs::create_directory(trimmed_path);
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
        handle_metadata_request(std::move(url_));
    }
private:
    // Position where the relative path of the cloned subfolder begins
    std::size_t pathb_idx_;

    std::string_view preprocess_folder(std::string_view folder) {
        std::size_t last_slash_idx = folder.rfind('/') + 1; 
        pathb_idx_ = folder.size() + 1;
        return folder.substr(last_slash_idx, folder.size() - last_slash_idx);
    }
};

class r_gitlab : public r_base {
public:
    r_gitlab(std::string_view author,
             std::string_view name,
             std::string_view branch,
             std::string_view folder)
        : r_base(author, name, branch, folder) {
        url_ = std::format("https://gitlab.com/api/v4/projects/{}%2F{}", author, name);
    }

    void start() override {
    }
};

#endif // __REPO_HPP__