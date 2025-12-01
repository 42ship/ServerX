## Directive: return

The `return` directive immediately stops processing the request and returns the specified status code to the client.

| | |
|---|---|
| **Syntax** | `return code [text];` or `return code URL;` or `return URL;` |
| **Default** | `---` |
| **Context** | `server`, `location`, `if` |

-----

## Description 💻

The **`return`** directive is used to **send a response to the client** without processing the request further through $\text{nginx}'s$ standard phases (like proxying or file serving).

  * **Behavior:** It's often used for **redirects**, implementing simple **access restrictions**, or handling **errors** by sending a specific HTTP status code. When $\text{nginx}$ encounters this directive, it immediately stops processing the current request.
  * **Arguments:**
      * **`code`**: A valid HTTP response status code.
          * If the code is **$301$** (Moved Permanently), **$302$** (Found), **$303$** (See Other), **$307$** (Temporary Redirect), or **$308$** (Permanent Redirect), the following argument must be a **URL** for the redirection.
          * For other codes (e.g., $204$, $400$, $403$, $404$, $500$, $503$), the following argument is optional and can be either a **URL** (which is returned in the response body) or **plain text** (which is also returned in the response body). If no text or URL is provided, $\text{nginx}$ uses its default message for the status code.
      * **`URL`**: A full or relative URI for redirection (for $3xx$ codes). If used alone, $\text{nginx}$ assumes a $302$ code.
      * **`text`**: The custom text to be returned in the response body for non-$3xx$ status codes.

-----

## Examples

### Example 1: Permanent Redirect (301)

This example demonstrates how to permanently redirect all requests from the old domain to the new domain.

```nginx
server {
    listen 80;
    server_name old.example.com;

    return 301 https://new.example.com$request_uri;
}
```

### Example 2: Access Restriction (403)

This example returns a $403$ Forbidden status for requests to the `/private/` path.

```nginx
server {
    listen 80;
    server_name example.com;

    location /private/ {
        return 403 "Access to this area is restricted.";
    }
}
```

### Example 3: Immediate Redirection (302)

This example redirects any request to the root (`/`) to a specific landing page using a temporary ($302$) redirect.

```nginx
server {
    listen 8080;

    location = / {
        return /landing-page.html;
    }
}
```
