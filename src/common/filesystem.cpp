#include "common/filesystem.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace utils {

namespace {
// Helper comparator for sorting (Directories first, then files alphabetically)
bool compareFileEntries(const FileEntry &a, const FileEntry &b) {
    if (a.isDir != b.isDir) {
        return a.isDir > b.isDir; // Directories first
    }
    return a.name < b.name; // Alphabetical order
}
} // namespace

bool writeFile(const std::string &content, const char *path) {
    std::ofstream out(path, std::ios::binary);

    if (!out.is_open()) {
        return false;
    }
    out.write(content.data(), content.size());
    bool success = out.good();
    out.close();
    return success;
}

bool writeFile(const std::string &content, const char *path, size_t start, size_t end) {
    if (start >= content.length() || end <= start) {
        return false;
    }

    size_t length = std::min(end - start, content.length() - start);

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }

    out.write(content.data() + start, static_cast<std::streamsize>(length));
    bool success = out.good();
    out.close();
    return success;
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
    size_t dotPos = fpath.rfind('.');
    if (dotPos != std::string::npos) {
        return fpath.substr(dotPos + 1);
    }
    return "";
}

TempFile::TempFile() : fd_(-1) {}
TempFile::~TempFile() { close(); }

bool TempFile::open() {
    if (fd_ != -1)
        close();
    char templateFileName[] = "/tmp/.webserv-file-body-XXXXXX";
    fd_ = mkstemp(templateFileName);
    if (fd_ < 1) {
        std::cerr << "Error: couldn't create a file for a request body: " << strerror(errno)
                  << std::endl;
        return false;
    }
    filePath_ = templateFileName;
    return true;
}

void TempFile::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    if (!filePath_.empty()) {
        unlink(filePath_.c_str());
        filePath_.clear();
    }
}

TempFile::operator int() const { return fd_; }

int TempFile::fd() const { return fd_; }
std::string const &TempFile::path() const { return filePath_; }
bool TempFile::isOpen() const { return fd_ != -1 && !filePath_.empty(); }

http::HttpStatus checkFileAccess(const std::string &path, int modeMask, bool allowDirectory) {
    struct stat statbuf;

    if (stat(path.c_str(), &statbuf) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            return http::NOT_FOUND;
        } else if (errno == EACCES) {
            return http::FORBIDDEN;
        }
        return http::INTERNAL_SERVER_ERROR;
    }

    if ((statbuf.st_mode & modeMask) == 0) {
        return http::FORBIDDEN;
    }

    // 3. Check if it is a directory
    if (S_ISDIR(statbuf.st_mode)) {
        if (!allowDirectory) {
            return http::FORBIDDEN;
        }
    }

    return http::OK;
}

std::string joinPaths(const std::string &p1, const std::string &p2) {
    if (p1.empty() || p1 == "/") {
        if (p2.empty() || p2[0] != '/')
            return "/" + p2;
        return p2;
    }
    if (p2.empty())
        return p1;
    std::string p1_clean = p1;
    if (p1_clean[p1_clean.length() - 1] == '/') {
        p1_clean.resize(p1_clean.length() - 1);
    }
    std::string p2_clean = p2;
    if (p2_clean[0] != '/')
        p2_clean = "/" + p2_clean;
    return p1_clean + p2_clean;
}

bool getFileStatus(std::string const &path, struct stat &buf) {
    return (stat(path.c_str(), &buf) == 0);
}

bool getDirectoryEntries(std::string const &path, std::vector<FileEntry> &entries) {
    entries.clear();

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    entries.reserve(64);
    const struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        if (name == "." || name == "..") {
            continue;
        }

        std::string fullPath = path;
        if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/') {
            fullPath += "/";
        }
        fullPath += name;

        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            FileEntry fe;
            fe.name = name;
            fe.isDir = S_ISDIR(st.st_mode);
            fe.mtime = st.st_mtime;
            fe.size = st.st_size;
            entries.push_back(fe);
        }
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end(), compareFileEntries);

    return true;
}

} // namespace utils
