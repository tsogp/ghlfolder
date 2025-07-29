#ifndef __MATCHER_HPP__
#define __MATCHER_HPP__

#include "repo.hpp"
#include <ctre.hpp>
#include <memory>
#include <optional>
#include <string_view>

namespace matcher {
std::unique_ptr<r_base> get_repo_data(std::string_view url, bool create_dir = false, std::optional<std::string_view> token = std::nullopt);
} // namespace matcher

#endif // __MATCHER_HPP__