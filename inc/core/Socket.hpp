#pragma once

#include <netdb.h>
#include <string>

namespace core {

class Socket {
public:
    Socket();
    explicit Socket(int port);
    Socket(std::string const &address, int port);
    ~Socket();

    int getFd(void) const;
    const struct sockaddr_in &getAddr(void) const;
    void bindTo(int port);
    void bindTo(std::string const &address, int port);

private:
    sockaddr_in addr_;
    int fd_;

    void createAndBind(std::string const &ipAddress, int port);
    Socket(Socket const &rhs);
    Socket const &operator=(Socket const &rhs);
};

} // namespace core
