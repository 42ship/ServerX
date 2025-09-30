#include "utils/utils.hpp"
#include <cerrno>
#include <sys/stat.h>

namespace utils {

bool writeFile(const std::string& content, const char* path) {
    std::ofstream out(path, std::ios::binary);

    if (!out.is_open()){
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

} // namespace utils
