#ifndef __TERM_HPP__
#define __TERM_HPP__

#include <sys/ioctl.h>
#include <unistd.h>

namespace term_data {
bool got_not_null_cols();
int get_width();
void hide_cursor();
void show_cursor();
}; // namespace term_data

#endif // __TERM_HPP__