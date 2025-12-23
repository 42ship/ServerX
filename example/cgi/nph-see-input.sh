#!/bin/bash

# --- NPH (Non-Parsed Header) CGI Script ---
# This script is responsible for *everything*, including
# the HTTP/1.1 status line. The server will not touch
# this output; it will send it raw to the client.

CLIENT_BODY=$(cat -)

RESPONSE_BODY=$(cat <<EOF
Hello from your NPH (Non-Parsed Header) CGI script!

This proves the 'cgi_nph on;' directive is working.
Your server did *not* parse this or add any headers (like 'Server' or 'Date').

Here are the environment variables I received:
$(env)

--- Body You Sent ---
$CLIENT_BODY
--- End of Body ---
EOF
)

BODY_LENGTH=$(echo -n "$RESPONSE_BODY" | wc -c)

# Status Line
printf "HTTP/1.1 200 OK\r\n"

# Headers
printf "Content-Type: text/plain\r\n"
printf "Content-Length: $BODY_LENGTH\r\n"
printf "X-NPH-Script: true\r\n"

# Blank line (CRLF) to end headers and begin body
printf "\r\n"

# Body
printf "%s" "$RESPONSE_BODY"

exit 0
