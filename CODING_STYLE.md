# Coding Style Guide

This document outlines the coding style for the `webserv` project.
All code must comply with the **C++98 standard** as required by the project subject.

## Automated Formatting

Use `clang-format` before committing:

## Naming Conventions

- **Classes & Structs:** `PascalCase`
  - _Example:_ `class Request;` `struct ServerConfig;`
- **Functions & Methods:** `camelCase`
  - _Example:_ `void parseRequest();`
- **Variables (Local & Member):** `camelCase`
- **Private Member Variables:** `camelCase` with a trailing underscore `_`.
  - _Example:_ `std::string clientAddress_;` `int port_;`
- **Constants:** `UPPER_CASE_SNAKE_CASE`. Use `const` for compile-time
  constants.
  - _Example:_ `const int MAX_CONNECTIONS = 1024;`

## C++98 Essentials

### Header Guards

Use `#pragma once` in all headers.

```cpp
  #pragma once

  #include <string>
  // ... rest of the header file
```

### Memory Management  

Implement Rule of Three for any class managing resources:
- Destructor
- Copy Constructor
- Copy Assignment Operator

### Include Order

1. Related header (for .cpp files)
2. System headers (`<iostream>`, etc.)
3. Project headers (`"config.hpp"`, etc.)

### Project Constraints

- C++98 standard library only
- No external libraries
- Non-blocking I/O with `poll()`/`select()`/`kqueue()`

## Code Quality

- Keep functions under 30 lines
- Use meaningful names
- Comment complex logic only
- Prefer RAII for resource management

To determine what a high quality of code, ask yourself "is the code readable, reusable, maintainable?" [see](https://en.wikipedia.org/wiki/SOLID)

Each class should have only one responsibility, each class has open/close principle.

See `DOCUMENTATION.md` for documentation standards.