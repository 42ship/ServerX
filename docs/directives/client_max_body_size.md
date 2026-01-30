# Directive: client_max_body_size

The `client_max_body_size` directive sets the maximum allowed size of the client request body.

## Syntax

```nginx
client_max_body_size size;
```

## Default

```nginx
client_max_body_size 1M;
```

## Context

- `server`
- `location`

## Description

If the size of a request exceeds the configured value, the `413 Request Entity Too Large` error is returned to the client.

The size can be specified in bytes (default), or using suffixes:
- `k` or `K` for kilobytes
- `m` or `M` for megabytes
- `g` or `G` for gigabytes

## Examples

```nginx
client_max_body_size 100; # 100 bytes
client_max_body_size 10k; # 10 kilobytes
client_max_body_size 100M; # 100 megabytes
```

### Example 1: Limit uploads to 10 MiB

```nginx
server {
    client_max_body_size 10M;
}
```

Any request larger than 10 MiB will be rejected with `413 Request Entity Too Large`.

### Example 2: Combined with upload_path

```nginx
server {
    client_max_body_size 5M;

    location /img/ {
        upload_path img/uploads;
    }
}
```

Uploads to /img/ will be stored in img/uploads and limited to 5 MiB.
