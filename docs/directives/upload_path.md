# Directive: upload_path

Specifies the destination directory where uploaded files should be saved.

|             |                                   |
| ----------- | --------------------------------- |
| **Syntax**  | `upload_path [path];`      |
| **Default** | `---`                             |
| **Context** | `location`                        |

---

## Description

The `upload_path` directive defines the directory on the server’s filesystem where files received via POST requests should be stored.
The path can be either absolute (starting with /) or relative to the server’s root directive.

If the specified directory does not exist or is not writable by the server process, the upload will fail with an appropriate HTTP error (403 or 500).

---

## Examples

### Example 1: Relative upload path (inside server root)

```nginx
server {
    root /var/www/html;

    location /img/ {
        upload_path img/uploads;
    }
}
```
Uploaded files to /img/ will be stored in /var/www/html/img/uploads/.

### Example 2: Absolute upload path

```nginx
location /upload/ {
    upload_path /data/uploads;
}
```

Files uploaded to /upload/ will be saved directly to /data/uploads/, regardless of the root setting.