#include "common/filesystem.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace utils {

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
    size_t dotPos = fpath.find('.');
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

} // namespace utils
