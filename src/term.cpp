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
void enable_virtual_terminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
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

void clear_line() {
#ifdef _WIN32
    enable_virtual_terminal();
#endif
    std::cout << "\033[2K";
}

void move_cursor_up(std::size_t l) {
    if (l != 0) {
#ifdef _WIN32
        enable_virtual_terminal();
#endif
        std::cout << "\033[" << l << "A";
    }
}

void move_cursor_down(std::size_t l) {
    if (l != 0) {
#ifdef _WIN32
        enable_virtual_terminal();
#endif
        std::cout << "\033[" << l << "B";
    }
}

void move_cursor_left(std::size_t count) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);

    COORD newPos = csbi.dwCursorPosition;
    newPos.X = std::max<SHORT>(0, newPos.X - static_cast<SHORT>(count));
    SetConsoleCursorPosition(hOut, newPos);
#else
    if (count != 0) {
        std::cout << "\033[" << count << "D";
    }
#endif
}

void move_cursor_right(std::size_t count) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);

    COORD newPos = csbi.dwCursorPosition;
    newPos.X += static_cast<SHORT>(count);
    SetConsoleCursorPosition(hOut, newPos);
#else
    if (count != 0) {
        std::cout << "\033[" << count << "C";
    }
#endif
}
} // namespace term_data
