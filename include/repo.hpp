#ifndef __REPO_HPP__
#define __REPO_HPP__

#include "bar_pool.hpp"
#include "thread_pool.hpp"
#include <cpr/api.h>
#include <cpr/cpr.h>
#include <cstddef>
#include <future>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

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
    std::optional<std::string_view> token;
    r_data data;

    r_base() = delete;
    r_base(std::string_view author,
           std::string_view name,
           std::string_view branch,
           std::string_view folder,
           std::optional<std::string_view> token,
            bool create_dir = false);

    r_base(const r_base &br) = default;
    r_base(r_base &&br) = default;
    r_base &operator=(const r_base &br) = default;
    r_base &operator=(r_base &&br) = default;

    virtual void start() = 0;
    virtual ~r_base() = default;

    void wait_for_all();
};

class r_github : public r_base {
public:
    r_github(std::string_view author,
             std::string_view name,
             std::string_view branch,
             std::string_view folder,
             std::optional<std::string_view> token,
            bool create_dir = false,
        bool from_zip = false);

    void handle_metadata_request(std::string url);
    void handle_request(const std::string &name, std::string url, unsigned int file_size);
    void start() override;
    void download_from_zip(std::string url);
private:
    // Position where the relative path of the cloned subfolder begins
    std::size_t pathb_idx_;
    std::string_view full_path;
    bool from_zip;
    std::string_view preprocess_folder(std::string_view folder);
};

class r_gitlab : public r_base {
public:
    r_gitlab(std::string_view author,
             std::string_view name,
             std::string_view branch,
             std::string_view folder,
             std::optional<std::string_view> token);

    void start() override;
};

#endif // __REPO_HPP__