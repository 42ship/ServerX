#include "Server.hpp"
#include <iostream>
#include <exception>

int main(void) {
    std::cout << "Starting production server with signal handling..." << std::endl;
    try {
        Server server;
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    std::cout << "Server shutdown complete" << std::endl;
    return 0;
}
