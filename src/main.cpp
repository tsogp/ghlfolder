#include "matcher.hpp"
#include <argparse/argparse.hpp>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sys/ioctl.h>
#include <optional>
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
    argparse::ArgumentParser program("ghlfolder");

    program.add_argument("url")
        .help("GitHub or GitLab subfolder URL")
        .required();

    program.add_argument("output_dir")
        .default_value(std::string("."))
        .help("Output directory for the subfolder contents");

    try {
        program.parse_args(argc, argv);

        std::string raw_url = program.get("url");
        std::string output_dir = program.get("output_dir");
        
        bool is_non_standard_dir = output_dir != ".";
        if (is_non_standard_dir) {
            if (!fs::create_directories(output_dir)) {
                if (!fs::is_empty(output_dir)) {
                    std::cerr << std::format("Error: output directory '{}' already exists and it is not empty.\n", output_dir);
                    exit(1);
                }
            } else if (!fs::is_directory(output_dir)) {
                std::cerr << std::format("Error: '{}' is not a directory.\n", output_dir);
                exit(1);
            } else if (!is_writable(output_dir)) {
                std::cerr << std::format("Error: output directory '{}' is not writable.\n", output_dir);
                exit(1);
            }

            fs::current_path(output_dir);

            // No endline character here because each new fetch bar starts with one
            std::cout << std::format("Cloning into '{}'...", output_dir);
        }

        // Create directory of subfolder name only if output_dir is not passed
        auto repo = matcher::get_repo_data(raw_url, !is_non_standard_dir);
        repo->start();
        repo->wait_for_all();
    } catch (const fs::filesystem_error &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }

    std::cout << "\nDone.\n";

    return 0;
}
