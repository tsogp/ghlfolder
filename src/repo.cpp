#include "repo.hpp"
#include "bar_pool.hpp"
#include "fetch_bar.hpp"
#include "thread_pool.hpp"
#include "utils.hpp"
#include "term.hpp"
#include "zip_file.hpp"
#include <cpr/api.h>
#include <cpr/bearer.h>
#include <cpr/cpr.h>
#include <cpr/cprtypes.h>
#include <cpr/error.h>
#include <cpr/response.h>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
    std::string random_string(std::string::size_type length) {
        static auto& chrs = "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        thread_local static std::mt19937 rg{std::random_device{}()};
        thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string s;
        s.reserve(length);

        while(length--) {
            s += chrs[pick(rg)];
        }

        return s;
    }

    void ensure_response_success(const cpr::Response& r) {
        // The only means of abortion by callback here is timeout
        if (r.error.code == cpr::ErrorCode::ABORTED_BY_CALLBACK) {
            throw std::runtime_error("\nTimeout");
        }

        if (r.status_code == 0) {
            throw std::runtime_error("\nError: " + r.error.message);
        } 
        
        if (r.status_code == 404) {
            throw std::runtime_error(
                "\nServer returned 404: does not exist. "
                "Check again if the repository URL is correct or you have access rights to it."
            );
        } 
        
        if (r.status_code == 401) {
            throw std::runtime_error(
                "\nServer returned 401: authorization required. "
                "Check again if the token that you have provided is correct."
            );
        } 
        
        if (r.status_code == 403 || r.status_code == 429) {
            throw std::runtime_error(
                "\nServer returned 403: not authorized. "
                "This might be due to no remaining quota for your IP. "
                "Consider adding a GitHub token with --token=<token> option."
            );
        }
    }

    constexpr int TIMEOUT_DURATION = 5; 

    bool is_timeouted(auto& last_progress_time, cpr::cpr_off_t downloadNow, unsigned int downloaded) {
        auto now = std::chrono::steady_clock::now();

        if (downloaded != downloadNow) {
            last_progress_time = now;
            downloaded = downloadNow;
        }

        return std::chrono::duration_cast<std::chrono::seconds>(now - last_progress_time).count() > 5;
    }
}

r_base::r_base(std::string_view author,
        std::string_view name,
        std::string_view branch,
        std::string_view folder,
        std::optional<std::string_view> token,
        bool create_dir)
    : data(author, name, branch, folder), token(token), worker_pool_(4) {
        
    if (create_dir) {
        if (fs::exists(folder)) {
            if (!fs::is_empty(folder)) {
                std::cout << std::format("Error: output directory '{}' already exists and it is not empty.\n", folder);
                std::exit(1);
            }
        } else if (!fs::create_directory(folder)) {
            std::cout << std::format("Error: couldn't create directory {}", folder);
            std::exit(1);
        }

        fs::current_path(folder);

        // No endline character here because each new fetch bar starts with one
        std::cout << std::format("Cloning into '{}'...", folder);
    };
}

r_github::r_github(std::string_view author,
            std::string_view name,
            std::string_view branch,
            std::string_view folder,
            std::optional<std::string_view> token,
        bool create_dir,
    bool from_zip)
    : r_base(author, name, branch, preprocess_folder(folder), token, create_dir), from_zip(from_zip), full_path(folder) {
    url_ = from_zip 
        ? std::format("https://api.github.com/repos/{}/{}/zipball/{}", author, name, branch)
        : std::format("https://api.github.com/repos/{}/{}/contents/{}?ref={}", author, name, folder, branch);
}

void r_github::handle_metadata_request(std::string url) {
    unsigned int downloaded = 0;
    auto last_progress = std::chrono::steady_clock::now();

    auto callback = cpr::ProgressCallback(
        [&](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow,
            cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow,
            intptr_t userdata) -> bool {
            if (is_timeouted(last_progress,  downloadNow, downloaded)) {
                return false;
            }

            return true;
        }
    );

    cpr::Response r = token
        ? cpr::Get(cpr::Url{std::move(url)}, cpr::Bearer{std::string(*token)}, callback)
        : cpr::Get(cpr::Url{std::move(url)}, callback);

    ensure_response_success(r);

    if (r.text.empty()) {
        return;
    }

    json data = json::parse(r.text);
    for (auto &it : data) {
        // TODO: fix to avoid copying
        std::string trimmed_path = it["path"].template get<std::string>().substr(pathb_idx_);
        if (it["type"] == "file") {
            auto task = std::packaged_task<bool(std::stop_token)>(
                [name = trimmed_path, url = it["download_url"], file_size = it["size"], this](std::stop_token stoken) {
                    return handle_request(name, url, file_size, stoken); 
                }
            );
            
            auto fut = task.get_future();
            futures_.emplace_back(std::move(fut));
            worker_pool_.push_job(std::move(task));
        } else if (it["type"] == "dir") {
            fs::create_directory(trimmed_path);
            handle_metadata_request(std::move(it["url"]));
        }
    }
}

bool r_github::handle_request(const std::string &name, std::string url, unsigned int file_size, std::stop_token stoken) {
    std::ofstream of(name, std::ios::binary);
    const std::size_t idx = bar_pool_.push_back(std::make_unique<fetch_bar>(name, file_size));
    unsigned int downloaded = 0;

    auto last_progress = std::chrono::steady_clock::now();
    auto callback = cpr::ProgressCallback(
        [&](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow,
            cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow,
            intptr_t userdata) -> bool {
            if (stoken.stop_requested()) {
                return false;
            }

            if (is_timeouted(last_progress,  downloadNow, downloaded)) {
                return false;
            }

            bar_pool_.tick_i(idx, (static_cast<double>(downloadNow) - downloaded) / file_size);
            downloaded = downloadNow;
            return true;
        }
    );

    cpr::Response r = token
        ? cpr::Download(of, cpr::Url{std::move(url)}, callback, cpr::Bearer{std::string(*token)})
        : cpr::Download(of, cpr::Url{std::move(url)}, callback);

    // If the pool is interrupted, return here
    if (stoken.stop_requested()) {
        return false;
    }

    ensure_response_success(r);
    
    // by here we know that downloading is completed
    if (downloaded < file_size) {
        bar_pool_.tick_i(idx, (static_cast<double>(file_size) - downloaded) / file_size);
    }

    return true;
}

void r_github::download_from_zip(std::string url) {
    unsigned int downloaded = 0;
    std::string temp_file_name = std::format("{}.zip", random_string(10));

    std::ofstream file(temp_file_name, std::ios::binary);
    if (!file.is_open()) {
        std::exit(-1);
    }

    std::cout << "\nDownloaded";

    auto last_progress = std::chrono::steady_clock::now();
    auto print_progress = cpr::ProgressCallback([&](cpr::cpr_off_t downloadTotal,
        cpr::cpr_off_t downloadNow,
        cpr::cpr_off_t uploadTotal,
        cpr::cpr_off_t uploadNow,
        intptr_t userdata) -> bool {
            if (stop_requested) {
                return false;
            }

            if (is_timeouted(last_progress,  downloadNow, downloaded)) {
                return false;
            }

            std::cout << std::right << std::setw(FETCH_BAR_SIZE_WIDTH) << utils::print_size(downloadNow);
            std::cout << "...";
            term_data::move_cursor_left(FETCH_BAR_SIZE_WIDTH + 3);
            return true;
        }
    );

    cpr::Response r = token
        ? cpr::Download(file, cpr::Url{std::move(url)}, print_progress, cpr::Bearer{std::string(*token)})
        : cpr::Download(file, cpr::Url{std::move(url)}, print_progress);
    
    file.close();

    // If the procedd is interrupted, return here
    if (stop_requested) {
        return;
    }

    ensure_response_success(r);
    
    std::cout << "\nDone.\nExtracting the archive...";

    try {
        zip_file zf(temp_file_name);
        zf.remove_unnecessary_dirs_and_save(full_path);
    } catch (const std::exception& e) {
        std::cout << e.what();
        std::exit(-1);
    }

    fs::remove(temp_file_name);
}

void r_github::start() {
    if (!from_zip) {
        handle_metadata_request(std::move(url_));
        worker_pool_.wait_for_all();
    } else {
        download_from_zip(std::move(url_));
    }
}

void r_github::stop() {
    stop_requested = true;
    if (!from_zip) {
        worker_pool_.stop_all();
    }
}

std::string_view r_github::preprocess_folder(std::string_view folder) {
    std::size_t last_slash_idx = folder.rfind('/') + 1; 
    pathb_idx_ = folder.size() + 1;
    return folder.substr(last_slash_idx, folder.size() - last_slash_idx);
}

r_gitlab::r_gitlab(std::string_view author,
            std::string_view name,
            std::string_view branch,
            std::string_view folder,
            std::optional<std::string_view> token)
    : r_base(author, name, branch, folder, token) {
    url_ = std::format("https://gitlab.com/api/v4/projects/{}%2F{}", author, name);
}

void r_gitlab::start() {}