#ifndef __REPO_HPP__
#define __REPO_HPP__

#include <format>
#include <string>

class r_base {
public:
    std::string_view author;
    std::string_view name;
    std::string_view branch;
    std::string_view folder;

    r_base() = delete;
    r_base(std::string_view author, std::string_view name, std::string_view branch, std::string_view folder)
        : author(author), name(name), branch(branch), folder(folder) {}

    r_base(const r_base &br) = default;
    r_base(r_base &&br) = default;
    r_base &operator=(const r_base &br) = default;
    r_base &operator=(r_base &&br) = default;

    virtual ~r_base() = default;

    [[nodiscard]] virtual std::string url() const = 0;
};

class r_github : public r_base {
public:
    r_github(std::string_view author, std::string_view name, std::string_view branch, std::string_view folder)
        : r_base(author, name, branch, folder) {
    }

    [[nodiscard]] std::string url() const override {
        return std::format("https://api.github.com/repos/{}/{}/contents/{}?ref={}", 
                           author, name, folder, branch);
    }
};

class r_gitlab : public r_base {
public:
    r_gitlab(std::string_view author, std::string_view name, std::string_view branch, std::string_view folder)
        : r_base(author, name, branch, folder) {
    }
    
    [[nodiscard]] std::string url() const override {
        return std::format("https://gitlab.com/api/v4/projects/{}%2F{}", 
                           author, name);
    }
};

#endif // __REPO_HPP__