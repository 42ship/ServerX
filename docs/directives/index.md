# Directive: index

Specifies the default file(s) to serve when a directory is requested.

|             |                                   |
| ----------- | --------------------------------- |
| **Syntax**  | `index [file1] [file2] ...;`      |
| **Default** | `---`                             |
| **Context** | `server`, `location`              |

---

## Description

The `index` directive defines one or more filenames that will be searched for and served when a client requests a directory URI.
If multiple files are specified, they are checked in order and the first existing file is served.

---

## Examples

### Example 1: Single index file

```nginx
location / {
    index index.html;
}
```

### Example 2: Multiple index files

```nginx
location / {
    index index.html