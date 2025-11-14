#!/usr/bin/env python3
import os

print("Content-Type: text/plain\r\n\r\n", end='')
print("=== CGI Environment Test ===\n")
print("GATEWAY_INTERFACE:", os.environ.get('GATEWAY_INTERFACE', 'NOT SET'))
print("SERVER_SOFTWARE:", os.environ.get('SERVER_SOFTWARE', 'NOT SET'))
print("SERVER_PROTOCOL:", os.environ.get('SERVER_PROTOCOL', 'NOT SET'))
print("\n=== All Environment Variables ===")
for key, value in sorted(os.environ.items()):
    print(f"{key}={value}")
