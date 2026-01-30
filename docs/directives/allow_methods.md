# allow_methods

The `allow_methods` directive restricts the allowed HTTP methods for a specific context.

## Syntax

```nginx
allow_methods [GET] [POST] [DELETE];
```

## Default

By default, all methods are allowed if the directive is not present.

## Context

- `server`
- `location`

## Description

Restricts the allowed HTTP methods. If a client sends a request using a method that is not in the allowed list, the server will respond with `405 Method Not Allowed`.

## Examples

```nginx
location / {
    allow_methods GET; # Only GET is allowed
}

location /post_body {
    allow_methods POST; # Only POST is allowed
}
```
