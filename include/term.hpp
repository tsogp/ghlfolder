#ifndef __TERM_HPP__
#define __TERM_HPP__

#include <cstddef>

namespace term_data {
bool got_not_null_cols();
int get_width();
void hide_cursor();
void show_cursor();
void clear_line();
void move_cursor_down(std::size_t l);
void move_cursor_up(std::size_t l);
void move_cursor_left(std::size_t count);
void move_cursor_right(std::size_t count);
}; // namespace term_data

#endif // __TERM_HPP__