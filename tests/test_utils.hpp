#pragma once

#include <sstream>
#include <string>

bool removeDirectoryRecursive(const std::string &path);

void writeFile(const std::string& content, const char* path);

void writeFile(const std::string &content, const char *path, int perm);

template <typename T> std::string toString(T const &v) {
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

template <typename T> T fromString(std::string const &str) {
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}