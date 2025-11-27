#pragma once

#include "config/ServerConfig.hpp"
#include "http/MimeTypes.hpp"
#include "http/Router.hpp"
#include "network/Acceptor.hpp"
#include "network/EventDispatcher.hpp"
#include <signal.h>
#include <vector>

namespace core {

/**
 * @class Server
 * @brief High-level server orchestrator that manages the complete server lifecycle.
 *
 * The Server class acts as a [Facade](https://en.wikipedia.org/wiki/Facade_pattern)
 * for the underlying network subsystem. It hides the complexity of the Reactor
 * pattern (e.g., EventDispatcher, Acceptors) behind a simple start() and stop()
 * interface.
 *
 * This class is the central point of control and is responsible for:
 * - Initializing all core components (EventDispatcher, Router, MimeTypes).
 * - Setting up listening sockets (Acceptors) based on the ServerConfig.
 * - Installing OS signal handlers (SIGTERM, SIGINT) to initiate a gracefulShutdown().
 * - Running the main event loop by calling EventDispatcher::handleEvents().
 * - Managing global, server-wide resources (like the /dev/null FD).
 *
 * It is implemented as a Singleton (using `instance_`) to allow the static
 * `signalHandler` to access the active Server instance and request a shutdown.
 */
class Server {
public:
    explicit Server(config::ServerConfig const &);
    ~Server();

    void start();
    void stop();
    bool getisRunning() const;
    void requestShutDown();

public:
    /**
     * @brief Gets the server-wide file descriptor for /dev/null.
     * @return A read-only file descriptor for /dev/null.
     * @note This is used by CGI to provide a valid, empty `stdin`
     * for requests that do not have a body.
     */
    static int getNullFd();

private:
    // clang-format off
    bool isRunning_; ///< Tracks main loop state (`true` between `start()` and `gracefulShutdown()`).
    volatile sig_atomic_t shutdownRequested_; ///< Graceful shutdown flag (must be `volatile sig_atomic_t` for signal handler).
    std::vector<network::Acceptor *> acceptors_; ///< Collection of active acceptor handlers (listening sockets).

    config::ServerConfig const &config_; ///< Const reference to the parsed server configuration.
    network::EventDispatcher &dispatcher_; ///< Reference to the central EventDispatcher (Reactor) singleton.
    http::MimeTypes mimeTypes_; ///< MIME types database instance (passed to the router).
    http::Router router_; ///< HTTP request router for dispatching requests to handlers.

    int nullFd_; ///< Server-wide file descriptor for `/dev/null` (opened at startup).
    // clang-format on

    void setupAcceptors();
    void setupSignalHandlers();
    void cleanup();
    void gracefulShutdown();

    Server(const Server &);
    Server &operator=(const Server &);
};

} // namespace core
