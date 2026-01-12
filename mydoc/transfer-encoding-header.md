## Keywords


- HTTP Transfer-Encoding
    - form of encoding
    - Request
    - Response header
- transfer messages
- nodes
- network

### Transfer-Encoding -> message

> What kinds of messages could be between two nodes

- A hop-by-hop header
    - a message
    - two nodes
- segment of connection
    - different Transfer-Encoding values

- end-to-end `Content-Encoding` header(rarely) Versus `chunked` (almost always)
    - compress data over the whole connection

Syntax

```
Transfer-Encoding: chunked
Transfer-Encoding: compress
Transfer-Encoding: deflate
Transfer-Encoding: gzip

// Several values can be listed, separated by a comma
Transfer-Encoding: gzip, chunked
```

```
7\r\n          ← "I'm sending 7 bytes"
Mozilla\r\n    ← the 7 bytes + \r\n
0\r\n          ← "I'm done"
\r\n           ← final terminator

```
