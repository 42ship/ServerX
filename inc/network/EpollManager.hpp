#pragma once

#include <stdint.h>
#include <sys/epoll.h>
#include <signal.h>

namespace network {

/**
 * @brief Synchronous event demultiplexer that wraps Linux epoll for I/O event monitoring.
 *
 * Provides a simplified interface to the epoll system call for detecting when file
 * descriptors are ready for I/O operations without blocking. Acts as the low-level
 * event detection mechanism in the Reactor pattern implementation. By "low-level", it means
 * epoll() is a system call to communnicate with the kernal space from the user space
 */
class EpollManager {
public:
    EpollManager();
    ~EpollManager();

    void addFd(int fd, uint32_t events);
    void removeFd(int fd);
    void modifyFd(int fd, uint32_t events);

    int waitForEvents(struct epoll_event *events, int max_events, int timeout = -1);

    void requestShutdown();
    bool getisShuttingDown() const;

private:
    int epollFd_;
    volatile sig_atomic_t isShuttingDown;

    EpollManager(const EpollManager &other);
    EpollManager &operator=(const EpollManager &other);
};

} // namespace network
