You are right to be confused, as the logic is subtle. Your existing `root` documentation is actually correct and *not* misguiding.

The key is that **`root`** always uses the *full* request path, while **`alias`** *replaces* part of the path.

  * **`root /var/www;`** + request `/page.html` = `/var/www/page.html`
  * **`alias /var/www/`** + `location /page.html` + request `/page.html` = `/var/www/page.html` (replaces `/page.html` with `/var/www/page.html`)

Let's use your templates to document `alias` and create the summary table entries.

-----

# Directive: alias

Sets a replacement filesystem path for a location.

| | |
| ----------- | ------------------------------------------------------- |
| **Syntax** | `alias [path];` |
| **Default** | `---` |
| **Context** | `location` |

-----

## Description

The `alias` directive defines a path to be used as the base for requests matching its `location` block.

Unlike the `root` directive, `alias` **replaces** the part of the URI that matches the `location` path. The part of the URI *after* the matching location path is then appended to the `alias` path.

This is extremely useful for serving files from a directory structure that does not match your URI structure.

**Important:** `alias` can only be used inside a `location` block.

-----

## Examples

### Example 1: Basic Usage

This example shows how to serve requests for `/media/` from a completely different directory, `/data/files/`.

```nginx
server {
    listen 8080;
    root /var/www/html; # A request for / will serve /var/www/html/index.html

    # A request for /media/video.mp4 will NOT use the server's root.
    # It will replace "/media/" with "/data/files/"
    #
    # Result: /data/files/video.mp4
    location /media/ {
        alias /data/files/;
    }
}
```

### Example 2: CGI Usage

This example shows how to map a CGI directory to a script folder.

```nginx
location /cgi-bin/ {
    # The request /cgi-bin/test.py
    # will be mapped to the script at:
    # /var/www/cgi-scripts/test.py

    alias /var/www/cgi-scripts/;
    cgi_pass /usr/bin/python3;
}
```
