#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace term_data {

int get_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 0;
#else
    winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
#endif
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

#ifdef _WIN32
static inline void enable_virtual_terminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Enable ANSI escape sequences
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

// Acknowledgement to https://github.com/p-ranav/indicators/blob/master/include/indicators/cursor_movement.hpp
static inline void move(int x, int y) {
    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!hStdout)
        return;

    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo(hStdout, &csbiInfo);

    COORD cursor;

    cursor.X = csbiInfo.dwCursorPosition.X + x;
    cursor.Y = csbiInfo.dwCursorPosition.Y + y;
    SetConsoleCursorPosition(hStdout, cursor);
}
#endif

void hide_cursor() {
#ifdef _WIN32
    enable_virtual_terminal();
#endif
    std::cout << "\033[?25l";
}

void show_cursor() {
#ifdef _WIN32
    enable_virtual_terminal();
#endif
    std::cout << "\033[?25h";
}

void move_cursor_up(int l) {
    if (l != 0) {
#ifdef _WIN32
        move(0, -l);
#else
        std::cout << "\033[" << l << "A";
#endif
    }
}

void move_cursor_down(int l) {
    if (l != 0) {
#ifdef _WIN32
        move(0, l);
#else
        std::cout << "\033[" << l << "B";
#endif
    }
}

void move_cursor_left(int l) {
    if (l != 0) {
#ifdef _WIN32
        move(-l, 0);
#else
        std::cout << "\033[" << l << "D";
#endif
    }
}

void move_cursor_right(int l) {
    if (l != 0) {
#ifdef _WIN32
        move(l, 0);
#else
        std::cout << "\033[" << l << "C";
#endif
    }
}
} // namespace term_data
