#include "curl_wrapper.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sys/ioctl.h>
#include <unistd.h>

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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <GitHub tree URL>\n";
        return 1;
    }

    std::string api_url = convert_github_url(argv[1]);

    curl_wrapper downloader(api_url);
    downloader.download_files();
    return 0;
}
