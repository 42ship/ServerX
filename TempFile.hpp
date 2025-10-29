#pragma once

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

class TempFile {
public:
    TempFile() : fd_(-1) {}

    ~TempFile() { close(); }

    bool open() {
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

    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        if (!filePath_.empty()) {
            unlink(filePath_.c_str());
            filePath_.clear();
        }
    }

    operator int() { return fd_; }

    int fd() const { return fd_; }
    std::string const &path() const { return filePath_; }
    bool isOpen() { return fd_ != -1 && !filePath_.empty(); }

private:
    int fd_;
    std::string filePath_;

    TempFile(TempFile const &TempFile);
    TempFile &operator=(TempFile const &rhs);
};
