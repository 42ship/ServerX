#include "config/ConfigException.hpp"
#include "core/Server.hpp"
#include "utils/Logger.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Config file should be provided");
        return 1;
    }
    try {
        core::Server server(argv[1]);
        server.start();
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return 2;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return 3;
    }
}
