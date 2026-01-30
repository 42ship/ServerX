*This project has been created as part of the 42 curriculum by dmelnyk, ogrativ, and szhong.*

# ServerX

ServerX is a high-performance, non-blocking HTTP/1.1 server implemented in **C++98**. It is designed to handle thousands of simultaneous connections using I/O multiplexing, providing a lightweight and modular alternative to traditional servers.

## Description

The goal of this project was to build a functional HTTP server from scratch, following the **RFC 2616** specifications. ServerX uses an event-driven architecture based on the **Reactor Pattern**, allowing it to process multiple requests efficiently without spawning a new thread for every client.

Key capabilities include:
- Serving static files (HTML, CSS, JS, images, etc.).
- Executing CGI scripts (Python, PHP, Bash) via `cgi_pass`.
- Handling file uploads and deletions.
- Managing multiple virtual hosts on different ports.

## Features

- **HTTP/1.1 Support**: Persistent connections (Keep-Alive), chunked request/response, and full header parsing.
- **Non-blocking I/O**: Built around a core `epoll` engine (Linux) for maximum efficiency.
- **Virtual Hosting**: Support for multiple server blocks using `listen` and `server_name` directives.
- **Dynamic Content**: Asynchronous CGI/1.1 support via the `cgi_pass` directive, allowing direct execution or interpreter mapping.
- **Smart Routing**: Match requests to locations based on prefix or exact match, with support for redirects and aliases.
- **File Management**: Built-in support for `POST` (upload) and `DELETE` methods, with configurable `upload_path`.
- **Customization**: User-defined error pages, client body limits (`client_max_body_size`), and directory listing (`autoindex`).
- **Graceful Shutdown**: Cleanly closes all connections and releases resources on signals (SIGINT, SIGTERM).

## Instructions

### Building the Project

Ensure you have a C++98-compatible compiler and `make` installed.

```bash
# Compile the server
make

# Clean object files
make clean

# Remove all build artifacts
make fclean
```

### Execution

Run the server by providing a configuration file:

```bash
# Run with custom configuration
./webserv config/example.conf

# Quick run (builds if needed)
make run ARGS="config/example.conf"
```

### Testing

Unit tests (if present) are in the `tests/` directory.

```bash
# Run tests
make test
```

## Project Structure

- `src/` — Source files (.cpp)
- `inc/` — Header files (.hpp)
- `config/` — Example configuration files
- `tests/` — Unit tests and test fixtures
- `mk/` — Makefile modules
- `docs/` — Additional documentation and directive guides
- `www/` — Default document root

## Usage Examples

### Valid Configuration Directives

- **`listen`**: address:port or port.
- **`server_name`**: domain names.
- **`root`** / **`alias`**: document root or path translation.
- **`index`**: default file to serve.
- **`allow_methods`**: restrict to `GET`, `POST`, or `DELETE`.
- **`cgi_pass`**: enable CGI or map to an interpreter.
- **`upload_path`**: directory for file uploads.
- **`client_max_body_size`**: limit request body size.
- **`autoindex`**: enable directory listing (`on`/`off`).
- **`error_page`**: custom error pages.
- **`return`**: HTTP redirects.

### Example Configuration

```nginx
server {
    listen 8080;
    server_name example.com;
    root www;
    index index.html;

    location /uploads/ {
        upload_path www/uploads;
        allow_methods GET POST DELETE;
        autoindex on;
    }

    location /cgi-bin/ {
        cgi_pass /usr/bin/python3; # Map all files here to python3
        allow_methods GET POST;
    }

    location /scripts/ {
        cgi_pass; # Execute scripts directly (using shebang)
    }
}
```

### Testing with Curl

**Get a static file:**
```bash
curl -i http://localhost:8080/index.html
```

**Upload a file:**
```bash
curl -X POST -H "Content-Type: text/plain" --data "Hello, ServerX!" http://localhost:8080/uploads/test.txt
```

**Run a CGI script:**
```bash
curl http://localhost:8080/cgi-bin/hello.py
```

## Technical Choices

### 1. Configuration Pipeline
Instead of a monolithic parser, we implemented a **Configuration Pipeline**. The process is split into several decoupled steps:
- **Lexer**: Tokenizes the configuration file.
- **Parser**: Builds an abstract tree of blocks and directives.
- **Validator**: Ensures configuration logic (e.g., no duplicate ports, correct contexts).
- **Mapper**: Converts the raw data into optimized runtime structures.
- **DirectiveHandlers**: Modular handlers that apply specific settings (e.g. `RootDirective`, `CgiPassDirective`).

### 2. Network Pipeline & I/O Multiplexing
The "Network Pipeline" is the heartbeat of ServerX. It follows an event-driven flow:
- **EpollManager**: A low-level wrapper around the Linux `epoll` system call. It monitors multiple file descriptors (FDs) simultaneously.
- **EventDispatcher**: A central singleton that dispatches events to registered handlers (`Acceptor`, `ClientHandler`, `CGIHandler`).
- **I/O Multiplexing**: While the core uses `epoll`, the interface is designed to be extensible (e.g., for `poll` or `select`).

### 3. HTTP Handling & Parsing
Our `RequestParser` is a **state-machine** based engine. It reads data incrementally, "feeding" on whatever bytes are currently available in the buffer. This handles large bodies and slow clients without consuming excess memory or threads.

### 4. CGI Integration
CGI execution is handled asynchronously to avoid blocking the main event loop. The server forks a process, manages pipes, and uses the `EventDispatcher` to stream output back to the client as it becomes available.

## Resources

- **HTTP/1.1 Specification**: [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) / [RFC 7230](https://tools.ietf.org/html/rfc7230)
- **CGI Specification**: [RFC 3875](https://tools.ietf.org/html/rfc3875)
- **Network Programming**: [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- **Linux Manuals**: [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html)
- **Design Inspiration**: [Nginx Documentation](https://nginx.org/en/docs/)

### AI Usage
Artificial Intelligence was utilized during development for:
- **Architecture**: Designing the modular directive system and event dispatcher.
- **Documentation**: Drafting README content and generating command snippets.
- **Refactoring**: Proposing cleaner helper functions and troubleshooting state-machine edge cases.
- **Testing**: Assisting in the creation of unit test scenarios.
