# Configuration for Unit Tests

For testing purposes, you can now construct a `ServerConfig` object programmatically without needing to parse a configuration file string. This makes tests cleaner, faster, and less prone to syntax errors.

### The New Approach: Programmatic Configuration

You can now build `ServerBlock` objects using a fluent API and add them directly to a `ServerConfig` instance.

**Example:**

```cpp
#include "config/ServerConfig.hpp"
#include "config/ServerBlock.hpp"

void run_my_test() {
    // 1. Create a ServerConfig object
    config::ServerConfig sc;

    // 2. Create and configure ServerBlocks
    config::ServerBlock first_server;
    first_server.port(8080)
                .root("/var/www/first/")
                .add("server_name", "first.com", "www.first.com");

    config::ServerBlock second_server;
    second_server.port(8080)
                 .root("/var/www/second/")
                 .add("server_name", "second.com");

    // 3. Add the server blocks to the config
    sc.addServer(first_server);
    sc.addServer(second_server);

    // 4. Use the ServerConfig object in your test...
    // Router router(sc, mime_types);
    // ...
}
```

### Deprecated Approach (For Reference)

Previously, creating a configuration for tests required writing a raw string and invoking the parser. This is no longer the recommended practice.

```cpp
// Old way: parsing a string
const std::string config_string = "server {\n"
                                  "  listen 8080;\n"
                                  "  server_name example.com;\n"
                                  "  location / { root /var/www; }\n"
                                  "}\n";

// The second argument `false` disables filesystem checks, which is useful for tests.
config::ServerConfig sc(config_string, false);
```

### ⚠️ Important Warning for Tests

**Do NOT declare `ServerConfig` objects as global or static variables in your test files.**

Creating a `ServerConfig` involves complex initialization of internal components (like the `DirectiveHandler` singleton). If multiple test threads or files attempt to initialize a global `ServerConfig` at the same time, it can lead to **race conditions** and unpredictable **segmentation faults**.

**Incorrect (causes race conditions):**

```cpp
// AVOID THIS IN YOUR TEST FILES
static config::ServerConfig my_global_config = create_test_config();

TEST_CASE("My Feature") {
    // ... uses my_global_config
}
```

**Correct:**

```cpp
// CORRECT: Create a new instance inside each test case or test group.
TEST_CASE("My Feature") {
    config::ServerConfig sc = create_test_config();
    // ... use the local `sc` object
}
```