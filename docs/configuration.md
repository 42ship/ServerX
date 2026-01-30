# Server Configuration (nginx-style)

Our server uses a configuration file syntax inspired by nginx.
The configuration is built from **directives**, which are organized into **blocks** (also called contexts).

## Main Blocks

- `server`: Defines a virtual server to handle requests.
- `location`: Defines how to process requests for specific URIs.
    A `location` block lives inside a `server` block.

## Location Matching

The server supports two types of location matching:

1.  **Prefix Matching** (Default):
    Matches the start of the URI. The longest matching prefix is chosen.
    ```nginx
    location /images/ { ... }
    ```
2.  **Extension Matching**:
    Matches the file extension at the end of the URI. Triggered by the `~` modifier and a pattern like `\.ext$`.
    Regex-like syntax is currently restricted to simple extension matching for performance and C++98 compatibility.
    ```nginx
    location ~ \.php$ { ... }
    ```
    *Note: Extension matches have priority over prefix matches (unless the prefix is an exact match).*


## Directives

Here is a list of all supported directives, their arguments, and the context in
which they can be used.

| Directive                                    | Context              | Brief Description                            |
| -------------------------------------------- | -------------------- | -------------------------------------------- |
| [`listen`](./directives/listen.md)           | `server`             | Specifies the port and address to listen on. |
| [`server_name`](./directives/server_name.md) | `server`             | Sets the names of the virtual server.        |
| [`root`](./directives/root.md)               | `server`, `location` | Sets the root directory for requests.        |
| [`index`](./directives/index.md)             | `location`, `server` | Sets the default file(s) to look for when a directory is requested. |
| [`upload_path`](./directives/upload_path.md) | `location` | Sets the upload directory for incoming file requests. |
| [`client_max_body_size`](./directives/client_max_body_size.md) | `server`, `location` | Defines the maximum allowed body size in bytes (default), or with k, m, g suffixes. |
| [`allow_methods`](./directives/allow_methods.md) | `server`, `location` | Restricts allowed HTTP methods (GET, POST, DELETE). |
| [`cgi_pass`](./directives/cgi_pass.md)       | `location` | Passes requests to a CGI script or interpreter. |
| [`alias`](./directives/alias.md) | `location` | Replaces a location's path with a new filesystem path. |
| [`error_page`](./directives/error_page.md) | `server`, `location` | Defines custom error pages for specific HTTP status codes. |
| [`return`](./directives/return.md) | `location`, `server` | Returns the specified status to the client. |
