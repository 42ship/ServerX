#include "common/filesystem.hpp"
#include <cerrno>
#include <fstream>
#include <sys/stat.h>

namespace utils {

bool writeFile(const std::string &content, const char *path) {
    std::ofstream out(path, std::ios::binary);

    if (!out.is_open()) {
        return false;
    }
    out.write(content.data(), content.size());
    out.close();
    return out.good();
}

const char *validateDirectoryPath(const char *path) {
    if (!path || *path == '\0') {
        return "path cannot be null or empty";
    }
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        switch (errno) {
        case ENOENT:
            return "does not exist";
        case EACCES:
            return "permission denied";
        case ENOTDIR:
            return "a component of the path prefix is not a directory";
        default:
            return "could not access path information";
        }
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        return "exists but is not a directory";
    }
    return NULL;
}

bool isDir(const std::string &p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string getFileExtension(const std::string &fpath) {
    size_t dotPos = fpath.find('.');
    if (dotPos != std::string::npos) {
        return fpath.substr(dotPos + 1);
    }
    return "";
}

} // namespace utils
