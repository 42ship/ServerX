# ChunkedBodyParser Validation Guide

Step-by-step instructions to validate the chunked transfer encoding implementation.

---

## Prerequisites

1. Build ServerX:
   ```bash
   cd ~/code/ServerX
   make
   ```

2. Ensure you have `nc` (netcat) installed for manual testing.

---

## Method 1: Unit Tests (Automated)

Run the existing unit tests:

```bash
cd ~/code/ServerX
make test
```

Look for `ChunkedBodyParser` test cases - all should pass.

---

## Method 2: Manual Testing with netcat

#j## Step 1: Start the Server

```bash
cd ~/code/ServerX
./webserv configs/default.conf
```

The server should start listening (check the config for the port, typically 8080).

### Step 2: Send a Chunked Request

Open another terminal and use netcat to send a chunked POST request:

```bash
nc localhost 8080
```

Then type (or paste) the following HTTP request:

```
POST /upload HTTP/1.1
Host: localhost
Transfer-Encoding: chunked

5
Hello
6
 World
0

```
or

```
 printf "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n" | nc localhost 9191
```
**Important formatting notes:**
- Each chunk size is in hexadecimal
- Each line ends with `\r\n` (press Enter)
- After the final `0` chunk, send an empty line

### Step 3: Verify Response

You should receive an HTTP response. If chunked parsing works:
- No `501 Not Implemented` error
- No `400 Bad Request` error (unless intentional)

---

## Method 3: Using curl

curl can send chunked requests automatically:

```bash
# Simple chunked upload
echo "Hello World" | curl -X POST \
  -H "Transfer-Encoding: chunked" \
  --data-binary @- \
  http://localhost:8080/upload -v
```

Or with a file:

```bash
curl -X POST \
  -H "Transfer-Encoding: chunked" \
  --data-binary @testfile.txt \
  http://localhost:8080/upload -v
```

---

## Method 4: Raw Chunked Data Script

Create a test script `test_chunked.sh`:

```bash
#!/bin/bash
# test_chunked.sh - Send raw chunked request

{
  printf "POST /upload HTTP/1.1\r\n"
  printf "Host: localhost\r\n"
  printf "Transfer-Encoding: chunked\r\n"
  printf "\r\n"
  printf "5\r\n"
  printf "Hello\r\n"
  printf "6\r\n"
  printf " World\r\n"
  printf "0\r\n"
  printf "\r\n"
} | nc localhost 8080
```

Run it:
```bash
chmod +x test_chunked.sh
./test_chunked.sh
```

---

## Method 5: Test Edge Cases

### A. Empty Body (Just Final Chunk)

```bash
{
  printf "POST /upload HTTP/1.1\r\n"
  printf "Host: localhost\r\n"
  printf "Transfer-Encoding: chunked\r\n"
  printf "\r\n"
  printf "0\r\n"
  printf "\r\n"
} | nc localhost 8080
```

Expected: Success with empty body.

### B. Large Chunk Size (Hex)

```bash
{
  printf "POST /upload HTTP/1.1\r\n"
  printf "Host: localhost\r\n"
  printf "Transfer-Encoding: chunked\r\n"
  printf "\r\n"
  printf "a\r\n"
  printf "0123456789\r\n"
  printf "0\r\n"
  printf "\r\n"
} | nc localhost 8080
```

Expected: Success, body = "0123456789" (10 bytes, 0xa in hex).

### C. Invalid Hex (Should Error)

```bash
{
  printf "POST /upload HTTP/1.1\r\n"
  printf "Host: localhost\r\n"
  printf "Transfer-Encoding: chunked\r\n"
  printf "\r\n"
  printf "XYZ\r\n"
  printf "data\r\n"
} | nc localhost 8080
```

Expected: `400 Bad Request` response.

### D. Body Size Limit

If `client_max_body_size` is set (e.g., 100 bytes), test exceeding it:

```bash
{
  printf "POST /upload HTTP/1.1\r\n"
  printf "Host: localhost\r\n"
  printf "Transfer-Encoding: chunked\r\n"
  printf "\r\n"
  printf "100\r\n"  # 256 bytes in hex
  # Generate 256 bytes of data
  head -c 256 /dev/zero | tr '\0' 'A'
  printf "\r\n"
  printf "0\r\n"
  printf "\r\n"
} | nc localhost 8080
```

Expected: `413 Payload Too Large` if limit is less than 256 bytes.

---

## Method 6: Debug with Logging

Enable trace logging in ServerX (if available):

```bash
# Check if LOG_TRACE is enabled in the build
# Look at utils/Logger.hpp for log levels

# Run with debug output
./webserv configs/default.conf 2>&1 | tee server.log
```

Watch for log messages like:
- `RequestParser::handleChunkedBody()`
- Chunk sizes being parsed
- Body bytes being written

---

## Expected Behavior Summary

| Test Case | Expected Response |
|-----------|-------------------|
| Valid chunked body | `200 OK` or appropriate success |
| Empty chunked body (just `0\r\n\r\n`) | Success |
| Invalid hex in chunk size | `400 Bad Request` |
| Missing CRLF after chunk | `400 Bad Request` |
| Body exceeds size limit | `413 Payload Too Large` |

---

## Troubleshooting

### Server returns 501 Not Implemented
- ChunkedBodyParser may not be integrated into RequestParser
- Check `src/http/RequestParser.cpp` handleChunkedBody()

### Server returns 400 Bad Request unexpectedly
- Check chunk format: `<hex-size>\r\n<data>\r\n`
- Ensure final chunk is `0\r\n\r\n`
- Verify no extra whitespace in chunk sizes

### Connection closes without response
- Server may have crashed
- Check server logs for errors
- Try running under gdb: `gdb ./webserv`

### Body content is incomplete
- Check if all chunks are being parsed
- Verify bytesConsumed is being tracked correctly
