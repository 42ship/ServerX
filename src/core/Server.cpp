#include "core/Server.hpp"
#include "config/ServerConfig.hpp"
#include "http/Router.hpp"
#include "network/EventDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace core {

namespace {

Server *instance_ = NULL;

void signalHandler(int sig) {
    if (instance_) {
        LOG_TRACE("Signal " << sig << " received. Initiating graceful shutdown.");
        instance_->requestShutDown();
        network::EventDispatcher::getInstance().requestShutdown();
    }
}

} // namespace

Server::Server(config::ServerConfig const &config)
    : isRunning_(false),
      shutdownRequested_(false),
      config_(config),
      dispatcher_(network::EventDispatcher::getInstance()),
      router_(config_, mimeTypes_),
      nullFd_(open("/dev/null", O_RDONLY)) {

    if (nullFd_ == -1) {
        throw std::runtime_error("Failed to open /dev/null: " + std::string(strerror(errno)));
    }

    instance_ = this;
    setupSignalHandlers();
    LOG_TRACE("Server instance created.");
}

Server::~Server() {
    cleanup();
    instance_ = NULL;
    if (nullFd_ >= 0) {
        close(nullFd_);
    }
    LOG_DEBUG("Server instance destroyed.");
}

void Server::setupSignalHandlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signalHandler;
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

void Server::start() {
    if (isRunning_) {
        LOG_WARN("Server::start() called, but server is already running.");
        return;
    }
    LOG_INFO("Starting server...");
    setupAcceptors();
    isRunning_ = true;

    LOG_INFO("Server is now running. Waiting for events...");
    LOG_INFO("Send SIGTERM (kill) or SIGINT (Ctrl+C) for graceful shutdown.");
    dispatcher_.handleEvents();
    gracefulShutdown();
}

void Server::stop() {
    LOG_TRACE("Server::stop() called.");
    requestShutDown();
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

bool Server::getisRunning() const { return isRunning_; }

void Server::setupAcceptors() {
    LOG_TRACE("Setting up server listeners (acceptors)...");
    config::ServerBlockMap const &servers = config_.getServersMap();
    for (config::ServerBlockMap::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        network::Acceptor *acceptor;
        if (it->second.size() > 1) {
            // Since we've got multiple servers Listening on same port then default address would be
            // "0.0.0.0" and all requests would be accepted and virtual server name matching would
            // decide which server block to get
            acceptor = new network::Acceptor(it->first, router_);
            LOG_INFO("Listening on 0.0.0.0:" << it->first);
        } else if (it->second.size() == 1) {
            acceptor = new network::Acceptor(it->second[0], router_);
            LOG_INFO("Listening on port: " << it->second[0].port());
        } else
            continue;
        dispatcher_.registerHandler(acceptor);
        acceptors_.push_back(acceptor);
    }
}

void Server::cleanup() {
    for (std::vector<network::Acceptor *>::iterator it = acceptors_.begin(); it != acceptors_.end();
         ++it) {
        dispatcher_.removeHandler(*it);
    }
    acceptors_.clear();
}

int core::Server::getNullFd() { return instance_->nullFd_; }

void Server::requestShutDown() { shutdownRequested_ = true; }

} // namespace core
