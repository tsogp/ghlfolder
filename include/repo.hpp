#ifndef __REPO_HPP__
#define __REPO_HPP__

#include <string>
#include <utility>

class r_base {
public:
    std::string author;
    std::string name;
    std::string branch;
    std::string folder;

    r_base(std::string author, std::string name, std::string branch, std::string folder)
        : author(std::move(author)), name(std::move(name)), branch(std::move(branch)), folder(std::move(folder)) {
    }

    r_base(const r_base &br) = default;
    r_base(r_base &&br) = default;
    r_base &operator=(const r_base &br) = default;
    r_base &operator=(r_base &&br) = default;

    virtual ~r_base() = default;

    [[nodiscard]] virtual std::string url() const = 0;
};

class r_github : public r_base {
public:
    r_github(std::string author, std::string name, std::string branch, std::string folder)
        : r_base(std::move(author), std::move(name), std::move(branch), std::move(folder)) {
    }

    [[nodiscard]] std::string url() const override {
        return "https://api.github.com/repos/" + author + "/" + name + "/contents/" + folder + "?ref=" + branch;
    }
};

class r_gitlab : public r_base {
public:
    r_gitlab(std::string author, std::string name, std::string branch, std::string folder)
        : r_base(std::move(author), std::move(name), std::move(branch), std::move(folder)) {
    }

    [[nodiscard]] std::string url() const override {
        return "https://gitlab.com/api/v4/projects/" + author + "%2F" + name;
    }
};

#endif // __REPO_HPP__