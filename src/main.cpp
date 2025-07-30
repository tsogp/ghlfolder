#include "matcher.hpp"
#include "term.hpp"
#include <argparse/argparse.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sys/ioctl.h>
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
    term_data::hide_cursor();

    argparse::ArgumentParser program("ghlfolder");

    program.add_argument("url").help("GitHub or GitLab subfolder URL").required();

    program.add_argument("--output_dir")
        .default_value(std::string("."))
        .help("Output directory for the subfolder contents");
    
    program.add_argument("--token")
        .help("GitHub or GitLab token");

    program.add_argument("--from_zip")
        .default_value(false)
        .implicit_value(true)
        .help("Download the full zip archive of the repository and the only keep the required folder");

    try {
        program.parse_args(argc, argv);

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }

    if (!term_data::got_not_null_cols()) {
        std::cerr << "Couldn't get the terminal width.";
        std::exit(1);
    }

    try {
        std::string raw_url = program.get("url");
        std::string output_dir = program.get("output_dir");
        std::optional<std::string> token = program.present("token");
        bool from_zip = program.get<bool>("--from_zip");

        bool is_non_standard_dir = output_dir != ".";
        if (is_non_standard_dir) {
            if (!fs::create_directories(output_dir)) {
                if (!fs::is_empty(output_dir)) {
                    std::cerr << std::format("Error: output directory '{}' already exists and it is not empty.\n",
                                             output_dir);
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
        auto repo = matcher::get_repo_data(raw_url, !is_non_standard_dir, token);
        repo->start();
        repo->wait_for_all();
    } catch (const fs::filesystem_error &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }

    std::cout << "\nDone.\n";
    term_data::show_cursor();
    return 0;
}
