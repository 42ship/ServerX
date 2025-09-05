#include <cerrno>
#include <csignal>
#include <iostream>
#include <exception>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/sendfile.h>
#include "ServerConfig.hpp"
#include "ConfigException.hpp"
#include "Socket.hpp"
#include "http/MimeTypes.hpp"
#include "http/Router.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

using config::ServerConfig;

using namespace std;

bool sendAll(int sockFd, char const *s, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        size_t just_sent = send(sockFd, s + sent, len - sent, 0);
        if (just_sent < 0) {
            std::cerr << "Coulnd send data: " << strerror(errno) << std::endl;
            return false;
        }
        sent += just_sent;
    }
    return true;
}

int main(int argc, char const **argv) {
    if (argc < 2) {
        std::cerr << "Error: Config file should be provided" << std::endl;
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);
    ServerConfig *cfg;
    MimeTypes mime("config/mime.types");
    try {
        cfg = new ServerConfig(argv[1]);
    } catch (config::ConfigException const &e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    };
    http::Router router(*cfg, mime);
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

        http::HttpResponse response = routres.handler.handle(request, routres);
        cout << "Sending back " << response << endl;

        std::string headers = response.buildHeaders();
        sendAll(client_fd, headers.c_str(), headers.size());
        switch (response.getBodyType()) {
        case http::BODY_FROM_FILE: {
            char buffer[8192];
            ssize_t bytes_read;
            while ((bytes_read = read(response.fileBody.fd, buffer, sizeof(buffer))) > 0) {
                if (!sendAll(client_fd, buffer, bytes_read))
                    break;
            }
            close(response.fileBody.fd);
            break;
        }
        case http::BODY_IN_MEMORY: {
            if (response.inMemoryBody.data && !response.inMemoryBody.data->empty()) {
                sendAll(client_fd, &(*response.inMemoryBody.data)[0],
                        response.inMemoryBody.data->size());
            }
            break;
        }
        case http::BODY_FROM_CGI: {
            break;
        }
        default: {
        }
        }
        if (response.getBodyType() == http::BODY_FROM_FILE) {
        }
        close(client_fd);
        std::cout << std::endl;
    }
}
