#!/bin/bash

# Headers
printf "Content-Type: application/octet-stream\r\n"
printf "Content-Length: 1073741824\r\n"
printf "Status: 202 OK\r\n"
printf "\r\n"

dd if=/dev/zero bs=1M count=1024 2>/dev/null
