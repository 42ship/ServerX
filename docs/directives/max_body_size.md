# Directive: max_body_size

Sets the maximum allowed file size (in MiB) for uploaded files.

|             |                                   |
| ----------- | --------------------------------- |
| **Syntax**  | `max_body_size [size_in_MiB];` |
| **Default** | `---`                             |
| **Context** | `server`                          |

---

## Description

The `max_body_size` directive limits the size of the uploaded file content.
If the size specified in the `Content-Length` header exceeds this limit, the server responds with `413 Payload Too Large`.

The size is expressed in mebibytes (MiB), where `1 MiB = 1,048,576 bytes`.

If this directive is not specified, uploads are unrestricted by default.

---

## Examples

### Example 1: Limit uploads to 10 MiB

```nginx
server {
    max_body_size 10;
}
```

Any upload larger than 10 MiB will be rejected with `413 Payload Too Large`.

### Example 2: Combined with upload_path

```nginx
server {

    max_body_size 5;

    location /img/ {
        upload_path img/uploads;
    }
}
```

Uploads to /img/ will be stored in img/uploads and limited to 5 MiB.
