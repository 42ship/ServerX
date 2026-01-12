#!/usr/bin/env python3

import sys
import time

# --- 1GB CGI Script with Content-Length ---

ONE_MB = 1024 * 1024
ONE_GB = 256 * ONE_MB
CHUNK_SIZE = 64 * 1024

print("Status: 200 OK")
print("Content-Type: application/octet-stream")
print(f"Content-Length: {ONE_GB}")
print('Content-Disposition: attachment; filename="1gb_file.dat"')
print(flush=True)

try:
    chunk = b"a" * CHUNK_SIZE

    total_sent = 0
    while total_sent < ONE_GB:
        print(chunk, end="")
        total_sent += len(chunk)
        time.sleep(1)

except BrokenPipeError:
    pass
except Exception as e:
    print(f"CGI Error: {e}", file=sys.stderr)
    pass
finally:
    sys.exit(0)
