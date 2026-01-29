#!/bin/bash

# Read the body from stdin
BODY=$(cat -)

# Prepare the response message
RESPONSE="Webserv CGI Echo Script
------------------------
Method:         $REQUEST_METHOD
Path:           $SCRIPT_NAME
Query String:   $QUERY_STRING
Remote Address: $REMOTE_ADDR
Content Length: $CONTENT_LENGTH

Body:
$BODY
"

# Calculate the precise byte length of the response
RESPONSE_LEN=$(printf "%s" "$RESPONSE" | wc -c)

echo "Content-Type: text/plain"
echo "Content-Length: $RESPONSE_LEN"
echo "Status: 200 OK"
echo ""

# Output the message without an extra trailing newline
printf "%s" "$RESPONSE"
