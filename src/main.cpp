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
    argparse::ArgumentParser program("ghlfolder");

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
