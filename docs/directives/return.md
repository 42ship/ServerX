# Directive: return

Stops processing and returns a specified response directly to the client.

| | |
| :--- | :--- |
| **Syntax** | `return code [text \| URL];` <br> `return URL;` |
| **Default** | `---` |
| **Context** | `server`, `location` |

-----

## Description

The `return` directive immediately stops the processing of a request and sends the specified response to the client. It's a powerful way to implement redirects or handle specific paths without needing to serve a file.

It can be used in one of the following forms:

  * **`return code;`**
    Returns the specified HTTP status code. For redirect codes (301, 302, 303, 307, 308), the response body will contain standard redirect text.

  * **`return code URL;`**
    Returns a redirect response. The `code` must be a valid redirect status code (301, 302, 303, 307, or 308). The `URL` specifies the target for the redirect.

  * **`return URL;`**
    A shortcut that returns a `302 Found` temporary redirect to the specified `URL`.

  * **`return code text;`**
    Returns the specified status code and sends the `text` as the response body. The `Content-Type` will typically be `text/plain`.

-----

## Examples

### Example 1: Permanent Redirect for an Entire Server

This is useful when you have decommissioned an old domain and want to redirect all traffic to a new one.

```nginx
server {
    listen 80;
    server_name old-domain.com;
    return 301 http://www.new-domain.com/;
}
```

### Example 2: Temporary Redirect for a specific location

Imagine you are temporarily moving a section of your site. This tells clients that requests to `/blog/` should go to a different server for now.

```nginx
location /blog/ {
    return 302 http://blog.example.com/;
}
```

### Example 3: Return a Custom "Under Construction" Message

For a part of your site that is not yet ready, you can return a `503 Service Unavailable` code with a plain text message.

```nginx
location /new-feature/ {
    return 503 "This feature is currently under construction. Please check back later.";
}
```

### Example 4: Forbidding Access to a Path

You can explicitly block access to a directory, such as `.git`, by returning a `403 Forbidden` error.

```nginx
location /.git/ {
    return 403;
}
```
