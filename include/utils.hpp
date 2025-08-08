#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>

namespace utils {
    void global_cleanup();
    std::string print_size(unsigned int bytes);
}

#endif // __UTILS_HPP__