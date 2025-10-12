#include "config/ServerConfig.hpp"
#include "config/internal/ConfigException.hpp"
#include "core/Server.hpp"
#include "utils/Logger.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Config file should be provided");
        return 1;
    }
    try {
        config::ServerConfig cfg(argv[1]);
        core::Server server(cfg);
        server.start();
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return 2;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return 3;
    }
}
