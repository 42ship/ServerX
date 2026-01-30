*This project has been created as part of the 42 curriculum by dmelnyk, ogrativ, szhong.*

# ServerX

## Description
ServerX is a lightweight, non-blocking HTTP/1.1 server implemented in C++98.  
The goal of the project is to build a small, standards-aware web server that handles multiple clients concurrently using I/O multiplexing and supports static file serving, CGI execution, and basic HTTP features required by the curriculum.

## Features
- HTTP/1.1 request parsing and response generation
- Non-blocking I/O with epoll/poll/select/kqueue (platform dependent)
- Multiple virtual server configurations (nginx-style)
- Static file serving with custom error pages
- CGI script execution
- Request timeout handling

## Requirements
- Linux (development and testing)
- C++98-compatible compiler (g++ or clang++)
- make
- Standard C++ library only (no external dependencies)

## Instructions

### Build
```bash
# from project root
make
```

### Clean
```bash
make clean
```

### Run
```bash
# Run with a custom configuration file
./webserv config/custom.conf

# Quick run (builds if needed)
make run ARGS="config/default.conf"
```

Configuration files use nginx-style syntax. See config/ for examples.

### Tests
Unit tests (if present) are in the tests/ directory. Run your test harness or inspect tests/ for per-project instructions:
```bash
# example (if a test runner is provided)
make test
```

## Project structure
- src/        — source files
- inc/        — headers
- config/     — example configuration files
- tests/      — unit tests and test fixtures
- mk/         — Makefile modules
- docs/       — additional documentation

## Usage examples
Serve files from a configured root and test with curl:
```bash
# start server with default config
./webserv config/default.conf &

# request a static file
curl -v http://localhost:8080/index.html
```

To test a CGI route, place an executable script in the configured CGI path and request its URL.

## Resources
Classic references and useful reading:
- HTTP/1.1: RFC 7230–7235 — https://tools.ietf.org/html/rfc7230
- CGI specification — https://tools.ietf.org/html/rfc3875
- epoll(7) manual and Linux man-pages — https://man7.org/linux/man-pages/
- Beej's Guide to Network Programming (sockets overview) — https://beej.us/guide/bgnet/
- MDN Web Docs — HTTP overview and headers — https://developer.mozilla.org/

AI usage disclosure:
- AI used: ChatGPT / GitHub Copilot.
- Tasks assisted: drafting and rewriting this README; generating example command snippets and suggested test scaffolding; proposing refactor ideas and small helper-function suggestions during development discussion.
- Not used for: implementing core server logic or final production-critical algorithms (all core implementation was written and reviewed by the project authors).

## Contributing
Follow the coding standards described in CODING_STYLE.md. Create issues or PRs for fixes and features. Include tests where applicable.

## License
This project is part of the 42 School curriculum. See LICENSE or contact the authors for