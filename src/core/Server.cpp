#include "core/Server.hpp"

#include <cstring>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include "http/Router.hpp"
#include "network/InitiationDispatcher.hpp"
#include "utils/Logger.hpp"

namespace core {

// TODO write the documentation
// In Server.hpp - this is a DECLARATION
// class Server {
//     static Server* instance_;  // ← Declaration: "instance_ exists somewhere"
// };
Server *Server::instance_ = NULL;

Server::Server(char const *fpath)
    : shutdownRequested_(false),
      isRunning_(false),
      config_(fpath),
      dispatcher_(network::InitiationDispatcher::getInstance()),
      router_(config_, mimeTypes_) {
    instance_ = this;
    setupSignalHandlers();
    LOG_INFO("Server instance created.");
}

Server::~Server() {
    cleanup();
    instance_ = NULL;
    LOG_INFO("Server instance destroyed.");
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
        throw std::runtime_error("Failed to install SIGHUP handler");
    }
    signal(SIGPIPE, SIG_IGN);
    LOG_DEBUG("Signal handlers installed.");
}

void Server::signalHandler(int sig) {
    if (instance_) {
        LOG_INFO("Signal " << sig << " received. Initiating graceful shutdown.");
        instance_->shutdownRequested_ = true;
        network::InitiationDispatcher::getInstance().requestShutdown();
    }
}

void Server::start() {
    if (isRunning_) {
        LOG_WARN("Server::start() called, but server is already running.");
        return;
    }
    LOG_INFO("Starting server...");
    try {
        setupAcceptors();
        isRunning_ = true;

        LOG_INFO("Server is now running. Waiting for events...");
        LOG_INFO("Send SIGTERM (kill) or SIGINT (Ctrl+C) for graceful shutdown.");
        dispatcher_.handleEvents();
        gracefulShutdown();
    } catch (const std::exception &e) {
        LOG_ERROR("Server startup failed: " << e.what());
        cleanup();
        throw;
    }
}

void Server::stop() {
    LOG_INFO("Server::stop() called.");
    shutdownRequested_ = true;
    dispatcher_.requestShutdown();
}

void Server::gracefulShutdown() {
    if (!isRunning_) {
        return;
    }
    LOG_INFO("Performing graceful shutdown...");
    cleanup();
    isRunning_ = false;
    LOG_INFO("Server shutdown complete.");
}

bool Server::getisRunning() const {
    return isRunning_;
}

void Server::setupAcceptors() {
    LOG_INFO("Setting up server listeners (acceptors)...");
    config::ServerBlockVec const &servers = config_.getServers();
    for (config::ServerBlockVec::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        network::Acceptor *acceptor = new network::Acceptor(*it, router_);
        dispatcher_.registerHandler(acceptor);
        acceptors_.push_back(acceptor);
        LOG_INFO("Listening on port " << it->getPort());
    }
}

void Server::cleanup() {
    for (std::vector<network::Acceptor *>::iterator it = acceptors_.begin(); it != acceptors_.end();
         ++it) {
        dispatcher_.removeHandler((*it)->getHandle());
    }
    acceptors_.clear();
}

} // namespace core
