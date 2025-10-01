#pragma once

#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace utils {

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

bool writeFile(const std::string& content, const char* path);

const char *validateDirectoryPath(const char *path);

inline bool isDir(const std::string &p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

} // namespace utils