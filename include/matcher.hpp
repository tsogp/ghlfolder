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

std::unique_ptr<r_base> get_repo_data(std::string_view url) {
    if (auto [_, author, repo, branch, folder] = github_pattern.match(url)) {
        return std::make_unique<r_github>(author.to_string(), repo.to_string(), branch.to_string(), folder.to_string());
    }

    if (auto [_, author, repo, branch, folder] = gitlab_pattern.match(url)) {
        return std::make_unique<r_gitlab>(author.to_string(), repo.to_string(), branch.to_string(), folder.to_string());
    }

    return nullptr;
}

} // namespace matcher

#endif // __MATCHER_HPP__