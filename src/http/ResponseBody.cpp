#include "http/ResponseBody.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace http {

//==================== NoBody ====================
ssize_t NoBody::read(char *, size_t) { return 0; }
size_t NoBody::size() const { return 0; }
bool NoBody::isDone() const { return true; };
int NoBody::getEventSourceFd() const { return -1; }
//==================== NoBody ====================

//==================== FileBody ====================

FileBody::FileBody(std::string const &fpath) : fd_(-1), size_(0), sent_(0) {
    struct stat statbuf;
    if (stat(fpath.c_str(), &statbuf) != 0)
        throw std::runtime_error("stat(" + fpath + ")::" + strerror(errno));
    size_ = statbuf.st_size;
    // TODO: before evaluation remove NONBLOCK
    fd_ = open(fpath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_ < 0)
        throw std::runtime_error("open(" + fpath + ", O_RDONLY)::" + strerror(errno));
    LOG_TRACE("FileBody::FileBody(" << fpath << "): opened for reading and about to send " << size_
                                    << " bytes")
}

FileBody::~FileBody() {
    if (fd_ != -1)
        close(fd_);
}

ssize_t FileBody::read(char *buffer, size_t size) {
    if (fd_ == -1)
        return 0;
    ssize_t read = ::read(fd_, buffer, size);
    sent_ += read;
    return read;
}

size_t FileBody::size() const { return size_; }
bool FileBody::isDone() const { return sent_ == size_; };
int FileBody::getEventSourceFd() const { return fd_; }

//==================== FileBody ====================

//==================== BodyInMemory ====================

BodyInMemory::BodyInMemory(std::string const &body) : body_(body), bytesRead_(0) {}

size_t BodyInMemory::size() const { return body_.length(); }
bool BodyInMemory::isDone() const { return bytesRead_ == body_.length(); };

ssize_t BodyInMemory::read(char *buffer, size_t size) {
    size_t bytesToRead = std::min(size, body_.length() - bytesRead_);
    std::memcpy(buffer, body_.data() + bytesRead_, bytesToRead);
    bytesRead_ += bytesToRead;
    return bytesToRead;
}
int BodyInMemory::getEventSourceFd() const { return -1; }

//==================== BodyInMemory ====================

//==================== BodyFromCgi ====================
BodyFromCgi::BodyFromCgi(int pipeFd) : fd_(pipeFd), isDone_(false) {
    if (fd_ < 0) {
        isDone_ = true;
        return;
    }
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

BodyFromCgi::~BodyFromCgi() {
    if (fd_ >= 0)
        close(fd_);
}
ssize_t BodyFromCgi::read(char *buffer, size_t size) {
    if (isDone_)
        return 0;
    ssize_t bytesRead = ::read(fd_, buffer, size);
    if (bytesRead > 0) {
        return bytesRead;
    }
    if (bytesRead == 0) {
        isDone_ = true;
        return 0;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return -1;
    }
    LOG_ERROR("BodyFromCgi::read: " << strerror(errno));
    isDone_ = true;
    return -1;
}
size_t BodyFromCgi::size() const { return 0; }
bool BodyFromCgi::isDone() const { return isDone_; };
int BodyFromCgi::getEventSourceFd() const { return fd_; }
//==================== BodyFromCgi ====================

} // namespace http
