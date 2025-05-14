#ifndef __MATCHER_HPP__
#define __MATCHER_HPP__

#include "repo.hpp"
#include <ctre.hpp>
#include <memory>
#include <string_view>

namespace matcher {

namespace {
using namespace ctre::literals;

constexpr auto github_pattern = R"((?:https?:\/\/)?github\.com/([^/]+)/([^/]+)/tree/([^/]+)/(.*))"_ctre;
constexpr auto gitlab_pattern =
    R"((?:https?:\/\/)?gitlab\.com/([^\/]+)/([^\/]+)/-/tree/([^\/]+)/([^?]+)(?:\?ref_type=heads)?)"_ctre;
} // namespace

std::unique_ptr<r_base> get_repo_data(std::string_view url, std::string_view path) {
    if (auto [whole, author, repo, branch, folder] = github_pattern.match(url); whole) {
        return std::make_unique<r_github>(author.to_view(), repo.to_view(), branch.to_view(), folder.to_view(), path);
    }

    if (auto [whole, author, repo, branch, folder] = gitlab_pattern.match(url); whole) {
        return std::make_unique<r_gitlab>(author.to_view(), repo.to_view(), branch.to_view(), folder.to_view(), path);
    }

    return nullptr;
}

} // namespace matcher

#endif // __MATCHER_HPP__