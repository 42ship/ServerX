#include "core/Server.hpp"

#include <cstring>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include "network/InitiationDispatcher.hpp"

namespace core {

// TODO write the documentation
// In Server.hpp - this is a DECLARATION
// class Server {
//     static Server* instance_;  // ← Declaration: "instance_ exists somewhere"
// };
Server *Server::instance_ = NULL;

Server::Server()
    : dispatcher_(network::InitiationDispatcher::getInstance()), isRunning_(false), config_(config),
      shutdownRequested_(false) {
    instance_ = this;
    setupSignalHandlers();
}

Server::~Server() {
    cleanup();
    instance_ = NULL;
}
void Server::setupSignalHandlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &Server::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        throw std::runtime_error("Failed to install SIGRTERM handler");
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        throw std::runtime_error("Failed to install SIGINT handler");
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        throw std::runtime_error("Failed to install SIGINT handler");
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        throw std::runtime_error("Failed to install SIGHUP handler");
    }
    signal(SIGPIPE, SIG_IGN);
    std::cout << "Signal handlers installed successfully" << std::endl;
}

void Server::signalHandler(int sig) {
    const char *signame;

    switch (sig) {
    case SIGTERM:
        signame = "SIGTERM";
        break;
    case SIGINT:
        signame = "SIGINT";
        break;
    case SIGHUP:
        signame = "SIGHUP";
        break;
    default:
        signame = "UNKNOWN";
        break;
    }
    write(STDERR_FILENO, "Received signal: ", 17);
    write(STDERR_FILENO, signame, strlen(signame));
    write(STDERR_FILENO, "\n", 1);
    if (instance_) {
        instance_->shutdownRequested_ = true;
        network::InitiationDispatcher::getInstance().requestShutdown();
    }
}

// TODO SIGHUP
void Server::handleSignal(int sig) {
    std::cout << "Processing signal: " << sig << std::endl;

    switch (sig) {
    case SIGTERM:
    case SIGINT:
        std::cout << "Graceful shutdown requested" << std::endl;
        gracefulShutdown();
        break;
    case SIGHUP:
        std::cout << "Configuration reload requested (not implemented)" << std::endl;
        break;
    default:
        std::cout << "Unhandled signal: " << sig << std::endl;
        break;
    }
}

void Server::start() {
    if (isRunning_) {
        std::cout << "Server is already running" << std::endl;
        return;
    }
    std::cout << "Starting server..." << std::endl;
    try {
        setupAcceptors();
        isRunning_ = true;

        std::cout << "Server started on ports " << BASE_PORT << " to "
                  << (BASE_PORT + MAX_PORTS - 1) << std::endl;
        std::cout << "Send SIGTERM (kill) or SIGINT (Ctrl+C) for graceful shutdown" << std::endl;
        dispatcher_.handleEvents();
        gracefulShutdown();
    } catch (const std::exception &e) {
        std::cerr << "Server startup failed: " << e.what() << std::endl;
        cleanup();
        throw;
    }
}

void Server::stop() {
    std::cout << "Stop requested" << std::endl;
    shutdownRequested_ = true;
    dispatcher_.requestShutdown();
}

void Server::gracefulShutdown() {
    if (!isRunning_) {
        return;
    }
    std::cout << "Performing graceful shutdown..." << std::endl;
    std::cout << "Stopping acceptors..." << std::endl;
    for (size_t i = 0; i < acceptors_.size(); ++i) {
        dispatcher_.removeHandler(acceptors_[i]->getHandle());
    }
    std::cout << "Waiting for existing connections to finish..." << std::endl;
    sleep(2);
    cleanup();
    isRunning_ = false;
    std::cout << "Graceful shutdown completed" << std::endl;
}

bool Server::getisRunning() const {
    return isRunning_;
}

// void Server::setupAcceptors() {
//     for (int i = 0; i < MAX_PORTS; ++i) {
//         int port = BASE_PORT + i;
//         network::Acceptor *acceptor = new network::Acceptor(port);
//         acceptors_.push_back(acceptor);
//         dispatcher_.registerHandler(acceptor);
//     }
// }

void Server::setupAcceptors() {
    const std::vector<config::ServerBlock> &servers = config_.getServer();

    std::set<int> uniquePorts;

    for (size_t i = 0; i < servers.size(); ++i) {
        uniquePorts.insert(servers[i].port_);
    }
    for (std::set<int>::const_iterator it = uniquePorts.begin(); it != uniquePorts.end(); ++it) {
        network::Acceptor *acceptor = new network::Acceptor(*it);
        acceptors_.push_back(acceptor);
        dispatcher_.registerHandler(acceptor);
    }
}

void Server::cleanup() {
    std::cout << "Cleaning up resources..." << std::endl;

    for (size_t i = 0; i < acceptors_.size(); ++i) {
        delete acceptors_[i];
    }
    acceptors_.clear();

    std::cout << "Cleanup completed" << std::endl;
}

} // namespace core
