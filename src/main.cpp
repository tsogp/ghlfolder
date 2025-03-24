#include "curl_wrapper.hpp"
#include <cmath>
#include <exception>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <argparse/argparse.hpp>
#include "matcher.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("program_name");
    
    program.add_argument("url")
        .help("GitHub or GitLab subfolder URL");

    program.add_argument("-o", "--output_dir")
        .default_value(std::string("."))
        .required()
        .help("Output directory for the subfolder contents");

    try {
        program.parse_args(argc, argv);

        std::string raw_url = program.get("url");
        std::string output_dir = program.get("--output_dir");

        auto repo = matcher::get_repo_data(raw_url);

        curl_wrapper downloader(repo->url());
        downloader.download_files();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }
    
    return 0;
}
