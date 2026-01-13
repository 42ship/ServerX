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
| [`index`](./directives/index.md)             | `location`           | ---                                          |
| [`upload_path`](./directives/upload_path.md) | `location` | Sets the upload directory for incoming file requests. |
| [`upload_file_size`](./directives/upload_file_size.md) | `server` | Defines the maximum allowed upload size in MiB. |
| [`alias`](./directives/alias.md) | `location` | Replaces a location's path with a new filesystem path. |
| [`error_page`](./directives/error_page.md) | `server`, `location` | Defines custom error pages for specific HTTP status codes. |
| [`return`](./directives/return.md) | `location`, `server` | Returns the specified status to the client. |
