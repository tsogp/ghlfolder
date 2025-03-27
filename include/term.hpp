#ifndef __TERM_HPP__
#define __TERM_HPP__

#include <sys/ioctl.h>
#include <unistd.h>

namespace term_data {
// TODO: fix to use interrupts to do the syscall
static int get_width() {
    winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
}
}; // namespace term_data

#endif // __TERM_HPP__