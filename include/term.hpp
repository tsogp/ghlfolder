#ifndef __TERM_HPP__
#define __TERM_HPP__

#include <cstddef>

namespace term_data {
bool got_not_null_cols();
int get_width();
void hide_cursor();
void show_cursor();
void move_cursor_down(int l);
void move_cursor_up(int l);
void move_cursor_left(int l);
void move_cursor_right(int l);
}; // namespace term_data

#endif // __TERM_HPP__