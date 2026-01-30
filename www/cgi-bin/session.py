#!/usr/bin/env python3
import os
import sys
import uuid
import json

def get_cookies():
    cookie_str = os.environ.get("HTTP_COOKIE", "")
    cookies = {}
    if cookie_str:
        for cookie in cookie_str.split(";"):
            if "=" in cookie:
                k, v = cookie.strip().split("=", 1)
                cookies[k] = v
    return cookies

def load_session(session_id):
    session_file = f"/tmp/webserv_session_{session_id}.json"
    if os.path.exists(session_file):
        try:
            with open(session_file, "r") as f:
                return json.load(f)
        except:
            pass
    return {"visits": 0}

def save_session(session_id, data):
    session_file = f"/tmp/webserv_session_{session_id}.json"
    with open(session_file, "w") as f:
        json.dump(data, f)

cookies = get_cookies()
session_id = cookies.get("sessionID")

if not session_id:
    session_id = str(uuid.uuid4())
    print(f"Set-Cookie: sessionID={session_id}; Path=/; HttpOnly")

session_data = load_session(session_id)
session_data["visits"] += 1
save_session(session_id, session_data)

print("Content-Type: text/html")
print("\r\n")

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Session Management Example</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <h1>Session Management</h1>
    <section>
        <h2>Your Session Info</h2>
        <p><strong>Session ID:</strong> <code>{session_id}</code></p>
        <p><strong>Visits this session:</strong> {session_data['visits']}</p>
        <hr>
        <p>This information is stored in a cookie and a temporary server-side file.</p>
        <a href="/cgi-bin/session.py" class="btn">Refresh to increment</a>
        <a href="/" class="btn btn-secondary">Back to Dashboard</a>
    </section>
</body>
</html>""")
