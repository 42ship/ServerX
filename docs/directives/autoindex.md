# Directive: autoindex

Enables or disables directory listing.

|             |                                                         |
| ----------- | ------------------------------------------------------- |
| **Syntax**  | `autoindex on | off;`                                  |
| **Default** | `off`                                                   |
| **Context** | `location`                                              |

---

## Description

The `autoindex` directive enables or disables the automatic generation of a directory listing for requests that map to a directory and for which no index file is found.

When `autoindex` is set to `on`, the server will generate an HTML page listing the contents of the directory. This listing includes:
- File and directory names (directories have a trailing slash)
- Modification times
- File sizes (for files only)

If an index file (configured via the `index` directive) exists in the directory, the server will prioritize serving that file instead of generating a directory listing.

---

## Examples

### Example 1: Basic Usage

Enabling directory listing for a specific location.

```nginx
server {
    listen 8080;
    
    location /downloads/ {
        root /var/www;
        autoindex on;
    }
}
```
