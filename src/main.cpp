#include "matcher.hpp"
#include <argparse/argparse.hpp>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

namespace fs = std::filesystem;

bool is_writable(const std::string &path) {
    auto test_file = fs::path(path) / "tempfile";
    std::ofstream ofs(test_file);
    if (!ofs) {
        return false;
    }

    fs::remove(test_file);
    return true;
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("program_name");

    program.add_argument("url").help("GitHub or GitLab subfolder URL");

    program.add_argument("-o", "--output_dir")
        .default_value(std::string("."))
        .required()
        .help("Output directory for the subfolder contents");

    try {
        program.parse_args(argc, argv);

        std::string raw_url = program.get("url");
        std::string output_dir = program.get("--output_dir");

        if (!fs::exists(output_dir)) {
            std::cerr << std::format("Error: output directory {} does not exist\n", output_dir);
        } else if (!fs::is_directory(output_dir)) {
            std::cerr << std::format("Error: {} is not a directory\n", output_dir);
        } else if (!is_writable(output_dir)) {
            std::cerr << std::format("Error: output directory {} is not writable\n", output_dir);
        }

        fs::current_path(output_dir);

        auto repo = matcher::get_repo_data(raw_url, output_dir);
        repo->start();
        repo->wait_for_all();
        // curl_wrapper downloader(repo->url());
        // downloader.download_files();

        // bar_pool bp(std::make_unique<fetch_bar>("1", 123123123U),
        //             std::make_unique<fetch_bar>("2", 123123123U),
        //             std::make_unique<fetch_bar>("3", 123123123U),
        //             std::make_unique<fetch_bar>("4", 123123123U),
        //             std::make_unique<fetch_bar>("5", 123123123U));

        // auto job = [&bp](std::size_t i) {
        //     while (true) {
        //         bp.tick_i(i, 0.1);
        //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //         if (bp.is_i_complete(i)) {
        //             break;
        //         }
        //     }
        // };

        // auto push_job = [&bp, &job] {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //     bp.push_back(std::make_unique<fetch_bar>("6", 123123123U));
        //     job(5);
        // };

        // std::jthread(job, 0);
        // std::jthread(job, 1);
        // std::jthread(job, 2);
        // std::jthread(job, 3);
        // std::jthread(job, 4);

        // thread_pool pool(2);

        // for (std::size_t i = 0; i < 5; ++i) {
        //     std::packaged_task<void()> task([=] { job(i); });
        //     pool.push_job(std::move(task));
        // }

        // std::packaged_task<void()> push_task([=] { push_job(); });
        // pool.push_job(std::move(push_task));

        // std::this_thread::sleep_for(std::chrono::seconds(5));
    } catch (std::filesystem::filesystem_error const &ex) {
        return -1;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }

    std::cout << "\nDone.\n";

    return 0;
}
