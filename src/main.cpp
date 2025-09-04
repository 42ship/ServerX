#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cerrno>
#include "Socket.hpp"
#include "http/ResponseContent.hpp"
#include "http/Response.hpp"
#include "http/ResponseSend.hpp"
#include "ServerConfig.hpp"

// static bool send_all(int fd, const char *data, size_t len) {
//     size_t sent = 0;
//     while (sent < len) {
//         ssize_t n = send(fd, data + sent, len - sent,
// #ifdef MSG_NOSIGNAL
//                          MSG_NOSIGNAL
// #else
//                          0
// #endif
//         );
//         if (n <= 0)
//             return false;
//         sent += static_cast<size_t>(n);
//     }
//     return true;
// }

using config::ServerConfig;

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
};

HttpRequest parse_request(const char *buffer) {
    HttpRequest req;
    std::istringstream iss(buffer);
    iss >> req.method >> req.path >> req.version;
    return req;
}

int main() {
    try {
        ServerConfig cfg("config/example.conf");
        Socket sk(9191);
        if (listen(sk.getFd(), 16) < 0) {
            perror("listen");
            return 1;
        }
        // MimeTypes *mime = new MimeTypes();
        std::cout << "Listening on 0.0.0.0:9191\n";

        for (;;) {
            struct sockaddr_in client_addr;
            std::memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(sk.getFd(), (sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                if (errno == EINTR)
                    continue;
                perror("accept");
                continue;
            }

            char buffer[2048];
            ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0) {
                perror("recv");
                close(client_fd);
                continue;
            }
            HttpRequest req = parse_request(buffer);
            std::string path = req.path.substr(0, req.path.find_last_of('/') + 1);
            std::string file = req.path.substr(req.path.find_last_of('/') + 1);
            buffer[bytes_received >= 0 ? bytes_received : 0] = '\0';
            std::cout << "\nRequest:\n" << buffer << "\n";

            // ResponseBuilder rb(9191, "localhost", cfg, mime);
            // rb.build(req.version, "keep-alive", req.method, path, file);
            // ResponseSend::sendAll(client_fd, rb.getResponse());

            close(client_fd);
        }
    } catch (const std::exception &e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}

// int main(void) {
//     Socket sk(9191);
//     listen(sk.getFd(), 5);
//     struct sockaddr_in client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     int client_fd = accept(sk.getFd(), (struct sockaddr *)&client_addr, &client_len);
//     char buffer[1024] = {0};
//     int bytes_received = recv(client_fd, buffer, 1024, 0);
//     buffer[bytes_received] = '\0';
//     std::cout << buffer;
//     close(client_fd);
//     return 0;
// }

// #include <iostream>
// #include <exception>
// #include "ServerConfig.hpp"
// #include "ConfigException.hpp"

// using config::ServerConfig;

// int main(int argc, char const **argv) {
//     if (argc < 2) {
//         std::cerr << "Error: Config file should be provided" << std::endl;
//         return 1;
//     }
//     try {
//         ServerConfig cfg(argv[1]);
//     } catch (config::ConfigException const &e) {
//         std::cerr << e.what() << std::endl;
//     }
//     catch (std::exception const &e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//     };
//     return 0;
// }