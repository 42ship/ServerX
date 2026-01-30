#!/bin/bash

ENV_OUT=$(env)
LEN=$(printf "%s" "$ENV_OUT" | wc -c)

echo "Content-Type: text/plain"
echo "Content-Length: $LEN"
echo "Status: 200 OK"
echo ""

printf "%s" "$ENV_OUT"
