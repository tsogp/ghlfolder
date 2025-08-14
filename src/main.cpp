#include "matcher.hpp"
#include "repo.hpp"
#include "term.hpp"
#include "utils.hpp"
#include <argparse/argparse.hpp>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
namespace fs = std::filesystem;

namespace {
    std::atomic<bool> stop_requested = false;
    std::optional<std::string> folder_to_delete;
    std::unique_ptr<r_base> repo = nullptr;

    void signal_handler(int signum) {
        stop_requested = true;

        if (repo) {
            repo->stop();
        }
    }
   
#ifdef _WIN32
    BOOL WINAPI ConsoleHandler(DWORD eventType) {
        switch (eventType) {
        case CTRL_C_EVENT:           // Ctrl + C
        case CTRL_BREAK_EVENT:       // Ctrl + Break
        case CTRL_CLOSE_EVENT:       // Closing console window
        case CTRL_LOGOFF_EVENT:      // User logs off
        case CTRL_SHUTDOWN_EVENT:    // System shutdown
            signal_handler(SIGTERM); 
            return TRUE;
        default:
            return FALSE;
        }
    }
#endif


    void handle_deadly_signals() {
#ifdef _WIN32
        if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
            std::exit(1);
        }
#else
        struct sigaction sa{};
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGINT, &sa, nullptr) == -1 || sigaction(SIGTERM, &sa, nullptr) == -1) {
            std::exit(1);
        }
#endif
    }

    bool is_writable(const std::string &path) {
        auto test_file = fs::path(path) / "tempfile";
        std::ofstream ofs(test_file);
        if (!ofs) {
            return false;
        }
    
        fs::remove(test_file);
        return true;
    }
};

int main(int argc, char *argv[]) {
    std::atexit(utils::global_cleanup);
    handle_deadly_signals();
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
                    std::exit(1);
                }
            } else if (!fs::is_directory(output_dir)) {
                std::cerr << std::format("Error: '{}' is not a directory.\n", output_dir);
                std::exit(1);
            } else if (!is_writable(output_dir)) {
                std::cerr << std::format("Error: output directory '{}' is not writable.\n", output_dir);
                std::exit(1);
            }

            folder_to_delete = output_dir;

            fs::current_path(output_dir);

            // No endline character here because each new fetch bar starts with one
            std::cout << std::format("Cloning into '{}'...", output_dir);
        }

        // Create directory of subfolder name only if output_dir is not passed
        repo = matcher::get_repo_data(raw_url, !is_non_standard_dir, token, from_zip);
        if (repo == nullptr) {
            std::cerr << "URL doesn't have the GitHub repository subfolder format. For the whole repositories use git instead.\n";
            std::exit(1);
        }

        if (!is_non_standard_dir) {
            folder_to_delete = repo->data.folder;
        }

        repo->start();

        if (stop_requested) {
            fs::current_path("..");
            fs::remove_all(*folder_to_delete);
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }

    if (stop_requested) {
        std::cout << "\nStopping...\n";
    } else {
        std::cout << "\nDone.\n";
    }

    repo.reset();

    term_data::show_cursor();
    return stop_requested ? 1 : 0;
}
