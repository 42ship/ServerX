# cgi_pass

The `cgi_pass` directive enables CGI processing for a location.

## Syntax

```nginx
cgi_pass;
cgi_pass interpreter;
cgi_pass extension interpreter;
```

## Context

- `location`

## Description

- **No arguments**: Enables CGI for the location. The server will attempt to execute the requested file directly as a script (requires an appropriate shebang).
- **One argument**: Specifies an interpreter (e.g., `/usr/bin/python3`) that will be used to execute all files in the location.
- **Two arguments**: Maps a specific file extension (e.g., `.bla`) to a dedicated interpreter or executable (e.g., `./cgi_tester`).

## Examples

```nginx
location /cgi-bin/ {
    cgi_pass; # Execute scripts directly
}

location ~ \.py$ {
    cgi_pass /usr/bin/python3; # Use python3 for all .py files
}

location ~ \.bla$ {
    cgi_pass .bla ./cgi_tester; # Use cgi_tester for .bla files
}
```
