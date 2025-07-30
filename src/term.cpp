#include <iostream>
#include <sys/ioctl.h>
#include <term.h>
#include <unistd.h>

namespace term_data {
int get_width() {
    winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
}

bool got_not_null_cols() {
    const int max_retries = 5;
    int retries = 0;
    int cols = 0;
    while ((cols = get_width()) == 0 && retries != max_retries) {
        ++retries;
    }
    return retries != max_retries;
}

void hide_cursor() {
    std::cout << "\033[?25l";
}

void show_cursor() {
    std::cout << "\033[?25h";
}

void clear_line() { 
    std::cout << "\033[2K"; 
}

}; // namespace term_data