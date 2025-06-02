#ifndef __TERM_HPP__
#define __TERM_HPP__

#include <sys/ioctl.h>
#include <unistd.h>

namespace term_data {
int get_width();
}; // namespace term_data

#endif // __TERM_HPP__