#!/usr/bin/env python3

import sys

# import time

# --- 10MB CGI Stream Test ---

MB = 1024 * 1024
TOTAL_SIZE = 1024 * MB
CHUNK_SIZE = 64 * 1024

print("Status: 200 OK")
print("Content-Type: application/octet-stream")
print(f"Content-Length: {TOTAL_SIZE}")
print('Content-Disposition: attachment; filename="test_stream_10mb.dat"')
print(flush=True)

try:
    chunk = b"X" * CHUNK_SIZE

    bytes_sent = 0
    while bytes_sent < TOTAL_SIZE:
        sys.stdout.buffer.write(chunk)
        bytes_sent += CHUNK_SIZE
        # time.sleep(0.01)

except BrokenPipeError:
    pass
except Exception:
    pass
finally:
    sys.exit(0)
