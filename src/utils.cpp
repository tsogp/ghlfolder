#include "utils.hpp"
#include "term.hpp"

void utils::global_cleanup() {
    term_data::show_cursor();
}