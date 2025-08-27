// #include "core/Server.hpp"

// int main(void) {
//     std::cout << "Starting production server with signal handling..." << std::endl;
//     try {
//         Server server;
//         server.start();
//     } catch (const std::exception &e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return (1);
//     }
//     std::cout << "Server shutdown complete" << std::endl;
#include <iostream>
#include <exception>
#include "config/ServerConfig.hpp"
#include "config/ConfigException.hpp"

using config::ServerConfig;

int main(int argc, char const **argv) {
    if (argc < 2) {
        std::cerr << "Error: Config file should be provided" << std::endl;
        return 1;
    }
    try {
        ServerConfig cfg(argv[1]);
    } catch (config::ConfigException const &e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    };
    return 0;
}
