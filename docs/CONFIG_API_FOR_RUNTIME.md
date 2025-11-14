# Runtime Configuration API for HTTP Handlers

This document explains how to access configuration data from within HTTP handler classes. This is a high-level guide; for a complete and detailed API reference, including all available methods and classes, please refer to the **Doxygen documentation**.

### Overview

The `http::Router` automatically finds the correct `server` and `location` blocks that match an incoming request. Your handler's `handle` method receives pointers to these read-only configuration blocks.

  - `config::ServerBlock const *server`: The configuration for the matched virtual server.
  - `config::LocationBlock const *location`: The configuration for the matched location within that server.

Both `ServerBlock` and `LocationBlock` inherit from `config::Block`, which provides the core methods for accessing directive values.

```cpp
// Example Handler
class StaticFileHandler : public IHandler {
public:
    Response handle(Request const &req, config::ServerBlock const *server,
                        config::LocationBlock const *location) const {
        // --- Use server and location objects here ---
    }
};
```

-----

### Accessing Directive Values (config::Block)

The following public methods are available on both `ServerBlock` and `LocationBlock` objects. Any `protected` methods in the `Block` class are reserved for internal configuration setup and should not be used at runtime.

#### `root()`

Returns the root path for the current context. The `location` block's root will automatically inherit from the `server` block if not explicitly set.

  - **Return type:** `std::string`

**Example:**

```cpp
// Get the root path for serving files.
std::string root_path = location->root(); // e.g., "/var/www/html"
```

#### `indexFiles()`

Returns the list of index files specified in the `index` directive.

  - **Return type:** `std::vector<std::string>`

**Example:**

```cpp
// Get the list of index files to look for.
std::vector<std::string> index_files = location->indexFiles(); // e.g., ["index.html", "index.htm"]
```

#### `get(key, request)`

Retrieves the values for a directive, evaluating any variables using the context of the current HTTP request. This is the primary method for accessing directives that support **variable interpolation** (e.g., `$uri`, `$host`).

  - **Return type:** `std::vector<std::string>`
  - **Arguments:**
    1.  `key`: The name of the directive (e.g., `"server_name"`).
    2.  `req`: A const reference to the `http::Request`.

**Example:**

```cpp
// Get all server names for the virtual host
std::vector<std::string> server_names = server->get("server_name", req);

// Example with a hypothetical directive using a variable
// config: `log_path /var/log/$host.log;`
std::string log_file = location->getFirstEvaluatedString("log_path", req); // -> "/var/log/example.com.log"
```

#### `getFirstEvaluatedString(key, request)`

Similar to `get`, but returns only the *first* value as a string. This is a convenience method for single-value directives and also supports variable interpolation.

  - **Return type:** `std::string`

**Example:**

```cpp
// If you only need the first server name
std::string primary_name = server->getFirstEvaluatedString("server_name", req);
```

#### `has(key)`

Checks if a directive is defined in the current block.

  - **Return type:** `bool`

**Example:**

```cpp
if (location->has("auth_basic")) {
    // Perform authentication
}
```

-----

### `config::ServerBlock` Specifics

In addition to the methods from `Block`, `ServerBlock` provides:

  - **`port()`**: Returns the listening port (e.g., `8080`).
  - **`address()`**: Returns the listening IP address (e.g., `"127.0.0.1"`).

-----

### `config::LocationBlock` Specifics

In addition to the methods from `Block`, `LocationBlock` provides:

  - **`path()`**: Returns the URI path this location matches (e.g., `"/images/"`).
  - **`hasCgiPass()`**: A convenience method to check if the `cgi_pass` directive is set for this location.