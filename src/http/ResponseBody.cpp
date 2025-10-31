#include "http/ResponseBody.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace http {

FileBody::FileBody(std::string const &fpath) : fd_(-1), size_(0) {
    struct stat statbuf;
    if (stat(fpath.c_str(), &statbuf) != 0)
        throw std::runtime_error("stat(" + fpath + ")::" + strerror(errno));
    size_ = statbuf.st_size;
    // TODO: before evaluation remove NONBLOCK
    fd_ = open(fpath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_ < 0)
        throw std::runtime_error("open(" + fpath + ", O_RDONLY)::" + strerror(errno));
}

FileBody::~FileBody() {
    if (fd_ != -1)
        close(fd_);
}

ssize_t FileBody::read(char *buffer, size_t size) {
    return (fd_ == -1 ? 0 : ::read(fd_, buffer, size));
}

BodyInMemory::BodyInMemory(std::string const &body) : body_(body), bytesRead_(0) {}

ssize_t BodyInMemory::read(char *buffer, size_t size) {
    size_t bytesToRead = std::min(size, body_.length() - bytesRead_);
    std::memcpy(buffer, body_.data() + bytesRead_, bytesToRead);
    return bytesToRead;
}

ssize_t BodyFromCgi::read(char *buffer, size_t size) {
    (void)buffer, (void)size, (void)fd_;
    return 0;
}

} // namespace http
