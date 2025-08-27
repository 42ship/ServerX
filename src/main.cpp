#include <iostream>
#include <exception>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "ServerConfig.hpp"
#include "ConfigException.hpp"
#include "Socket.hpp"
#include "http/Router.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/ResponseBuilder.hpp"

using config::ServerConfig;

using namespace std;

int main(int argc, char const **argv) {
    if (argc < 2) {
        std::cerr << "Error: Config file should be provided" << std::endl;
        return 1;
    }
    ServerConfig *cfg;
    try {
        cfg = new ServerConfig(argv[1]);
    } catch (config::ConfigException const &e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    };
    http::Router router(*cfg);
    Socket sk(9191);
    listen(sk.getFd(), 5);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int client_fd = accept(sk.getFd(), (struct sockaddr *)&client_addr, &client_len);
        char buffer[10240] = {0};
        int bytes_received = recv(client_fd, buffer, 10240, 0);
        buffer[bytes_received] = '\0';

        http::HttpRequest request = http::HttpRequest::parse(buffer);
        cout << request << endl;

        http::RouterResult routres = router.route(9191, request);
        cout << "Router result: " << (routres.server == NULL ? "Server not matched" : "Server hit")
             << ";\t" << (routres.location == NULL ? "Location not matched" : "Location hit")
             << endl;

        http::HttpResponse respsone = routres.handler.handle(request, routres);
        cout << "Sending back " << respsone << endl;

        std::string response = http::ResponseBuilder::build(respsone);
        send(client_fd, response.c_str(), response.size(), 0);
        close(client_fd);
        std::cout << std::endl;
    }
}
