#pragma once

#include "config/ServerConfig.hpp"
#include "http/MimeTypes.hpp"
#include "http/Router.hpp"
#include "network/Acceptor.hpp"
#include "network/InitiationDispatcher.hpp"
#include <signal.h>
#include <vector>

namespace core {

/**
 * @brief High-level server orchestrator that manages the complete server lifecycle.
 *
 * The Server class coordinates multiple acceptors, handles graceful shutdown, and provides a
 * unified interface for starting/stopping the event-driven TCP/IP server. Acts as the application's
 * main entry point into the Reactor pattern infrastructure.
 *
 * Basically, the Server class serves as application controller above the Reactor the design pattern
 * components and use them to build a complete server application. The Server class is like a bridge
 * between "I want to run a server" and "Here is how the Reactor pattern works" The design pattern
 * is Facade Pattern where it hides the complexity of "create dispatcher, create acceptors, register
 * handlers, start event loop, handle signals, cleanup" behind simple start() and stop() methods
 *
 * void start() will set up acceptors and start event loop
 * void stop() will gracefully shutdown the server instance in OS
 * void handleSignal(int sig) will handle OS signals because when you run a server in a OS, \
 * you run the server instance on the background. For example,
 * ```
 * $ ./server &
 * [1] 12345
 * Starting production server with signal handling ...
 * Server started on ports 2000 to 2019
 *
 * $ kill 12345
 * Received signal: SIGTERM
 * Performing graceful shutdown...
 * Stopping acceptors...
 * Waiting for existing connections to finish...
 * Cleaning up resources...
 * Server shutdown complete
 * ```
 *
 * void getisRunning() is the getter function to check server status
 *
 * static void signalHandler(int sig) static signal handler required by signal api
 *
 * bool shutdownRequested_ is a flag
 *
 *  static Server* instance_;  // Static pointer for signal handler access
 *  void setupAcceptors();     // Create listening sockets for multiple ports
 *  void setupSignalHandlers(); // Install signal handlers
 *  void cleanup();            // Clean up resources
 *  void gracefulShutdown();   // Perform graceful shutdown
 *
 *  Server(const Server&); // Prevent copying
 *  Server& operator=(const Server&); // Prevent copying
 */
class Server {
public:
    Server(config::ServerConfig const &);
    ~Server();

    void start();
    void stop();
    void handleSignal(int sig);
    bool getisRunning() const;

    static void signalHandler(int sig);

private:
    volatile sig_atomic_t shutdownRequested_;
    bool isRunning_;
    static Server *instance_;
    std::vector<network::Acceptor *> acceptors_;

    config::ServerConfig const &config_;
    network::InitiationDispatcher &dispatcher_;
    http::MimeTypes mimeTypes_;
    http::Router router_;

    void setupAcceptors();
    void setupSignalHandlers();
    void cleanup();
    void gracefulShutdown();

    Server(const Server &);
    Server &operator=(const Server &);
};

} // namespace core
