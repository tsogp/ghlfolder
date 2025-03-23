#include "curl_wrapper.hpp"
#include <cmath>
#include <exception>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sys/ioctl.h>
#include <unistd.h>
#include <argparse/argparse.hpp>

std::string convert_github_url(const std::string &url) {
    std::regex pattern(R"(https://github\.com/([^/]+)/([^/]+)/tree/([^/]+)/(.*))");
    std::smatch match;

    if (std::regex_match(url, match, pattern) && match.size() == 5) {
        std::string author = match[1];
        std::string repo = match[2];
        std::string branch = match[3];
        std::string folder = match[4];

        std::cout << "Cloning into " << folder << "...\n";
        std::filesystem::create_directory(folder);

        return "https://api.github.com/repos/" + author + "/" + repo + "/contents/" + folder + "?ref=" + branch;
    } 

    return "Invalid GitHub tree URL";
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("program_name");
    
    program.add_argument("url")
        .help("GitHub or GitLab subfolder URL");

    program.add_argument("o", "--output_dir")
        .default_value(std::string("."))
        .required()
        .help("Output directory for the subfolder contents");

    try {
        program.parse_args(argc, argv);

        std::string raw_url = program.get("url");
        std::string output_dir = program.get("--output_dir");

        std::string clean_url = convert_github_url(raw_url);

        curl_wrapper downloader(clean_url);
        downloader.download_files();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }
    
    return 0;
}
