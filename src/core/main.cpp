#include "config/ConfigException.hpp"
#include "core/Server.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Error: Config file should be provided" << std::endl;
        return 1;
    }
    try {
        core::Server server(argv[1]);
        server.start();
    } catch (config::ConfigException const &e) {
        std::cerr << e.what() << std::endl;
        return 2;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 3;
    }
    std::cout << "Server shutdown complete" << std::endl;
}
