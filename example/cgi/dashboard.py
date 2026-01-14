#!/usr/bin/env python3
import os
import sys
import uuid
import json
from datetime import datetime

# --- Helper Functions ---

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
    session_file = f"/tmp/session_{session_id}.json"
    if os.path.exists(session_file):
        with open(session_file, "r") as f:
            return json.load(f)
    return {"visits": 0, "last_visit": "Never"}

def save_session(session_id, data):
    session_file = f"/tmp/session_{session_id}.json"
    with open(session_file, "w") as f:
        json.dump(data, f)

# --- Main Logic ---

cookies = get_cookies()
session_id = cookies.get("sessionID")
theme = cookies.get("theme", "light")

# Initialize Session
if not session_id:
    session_id = str(uuid.uuid4())
    print(f"Set-Cookie: sessionID={session_id}; Path=/; HttpOnly")

# Update visits
session_data = load_session(session_id)
session_data["visits"] += 1
prev_visit = session_data["last_visit"]
session_data["last_visit"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
save_session(session_id, session_data)

# Handle theme toggle
query = os.environ.get("QUERY_STRING", "")
if "toggle_theme=1" in query:
    theme = "dark" if theme == "light" else "light"
    print(f"Set-Cookie: theme={theme}; Path=/; Max-Age=3600")
    # Redirect to clear query string
    print("Status: 303 See Other")
    print("Location: dashboard.py")
    print("\r\n")
    sys.exit(0)

# --- HTML Generator ---

bg_color = "#1a1a1a" if theme == "dark" else "#f4f4f9"
card_color = "#2d2d2d" if theme == "dark" else "#ffffff"
text_color = "#e0e0e0" if theme == "dark" else "#333333"
accent_color = "#4f46e5"

html_body = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Premium Session Dashboard</title>
    <style>
        body {{
            background-color: {bg_color};
            color: {text_color};
            font-family: 'Inter', system-ui, -apple-system, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            transition: all 0.3s ease;
        }}
        .card {{
            background: {card_color};
            padding: 2.5rem;
            border-radius: 1.5rem;
            box-shadow: 0 10px 25px rgba(0,0,0,0.1);
            width: 400px;
            text-align: center;
        }}
        h1 {{ margin-bottom: 0.5rem; font-weight: 700; }}
        .stat-grid {{
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1rem;
            margin: 2rem 0;
        }}
        .stat-item {{
            background: rgba(128, 128, 128, 0.1);
            padding: 1rem;
            border-radius: 1rem;
        }}
        .stat-value {{ font-size: 1.5rem; font-weight: bold; color: {accent_color}; }}
        .stat-label {{ font-size: 0.8rem; opacity: 0.7; margin-top: 0.2rem; }}
        .btn {{
            background: {accent_color};
            color: white;
            border: none;
            padding: 0.8rem 1.5rem;
            border-radius: 0.8rem;
            cursor: pointer;
            font-weight: 600;
            text-decoration: none;
            display: inline-block;
            transition: transform 0.2s;
        }}
        .btn:hover {{ transform: translateY(-2px); }}
        .session-id {{
            font-family: monospace;
            font-size: 0.7rem;
            background: rgba(0,0,0,0.05);
            padding: 0.5rem;
            border-radius: 0.5rem;
            word-break: break-all;
            margin-top: 1.5rem;
        }}
    </style>
</head>
<body>
    <div class="card">
        <h1>Dashboard</h1>
        <p>Welcome back to your persistent session</p>
        
        <div class="stat-grid">
            <div class="stat-item">
                <div class="stat-value">{session_data['visits']}</div>
                <div class="stat-label">Total Visits</div>
            </div>
            <div class="stat-item">
                <div style="font-size: 1rem; font-weight: bold;">{theme.upper()}</div>
                <div class="stat-label">Active Theme</div>
            </div>
        </div>
        
        <p style="font-size: 0.9rem; margin-bottom: 2rem;">
            Last visited: <br><strong>{prev_visit}</strong>
        </p>

        <a href="?toggle_theme=1" class="btn">Toggle { 'Light' if theme == 'dark' else 'Dark' } Mode</a>
        
        <div class="session-id">
            SID: {session_id}
        </div>
    </div>
</body>
</html>
"""

print("Content-Type: text/html")
print(f"Content-Length: {len(html_body.encode('utf-8'))}")
print("\r\n")
print(html_body)
