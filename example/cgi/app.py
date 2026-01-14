#!/usr/bin/env python3
import os
import sys
import uuid
import json
import urllib.parse
from datetime import datetime

# --- Configuration ---
SESSION_DIR = "/tmp"
SESSION_COOKIE = "sessionID"
THEME_COOKIE = "theme"

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

def load_session(sid):
    if not sid: return None
    path = os.path.join(SESSION_DIR, f"sess_{sid}.json")
    if os.path.exists(path):
        with open(path, "r") as f:
            return json.load(f)
    return None

def save_session(sid, data):
    path = os.path.join(SESSION_DIR, f"sess_{sid}.json")
    with open(path, "w") as f:
        json.dump(data, f)

def delete_session(sid):
    path = os.path.join(SESSION_DIR, f"sess_{sid}.json")
    if os.path.exists(path):
        os.remove(path)

def get_post_data():
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length > 0:
            return urllib.parse.parse_qs(sys.stdin.read(content_length))
    except:
        pass
    return {}

# --- Logic Implementation ---
cookies = get_cookies()
sid = cookies.get(SESSION_COOKIE)
theme = cookies.get(THEME_COOKIE, "dark") # Default to dark
session = load_session(sid)
query = urllib.parse.parse_qs(os.environ.get("QUERY_STRING", ""))

# 1. Handle Theme Toggle
if "toggle_theme" in query:
    theme = "light" if theme == "dark" else "dark"
    print(f"Set-Cookie: {THEME_COOKIE}={theme}; Path=/; Max-Age=31536000")
    print("Status: 303 See Other")
    print("Location: app.py")
    print("\r\n")
    sys.exit(0)

# 2. Handle Logout
if "logout" in query:
    if sid:
        delete_session(sid)
    print(f"Set-Cookie: {SESSION_COOKIE}=; Path=/; Max-Age=0")
    print("Status: 303 See Other")
    print("Location: app.py")
    print("\r\n")
    sys.exit(0)

# 3. Handle Login (POST)
if os.environ.get("REQUEST_METHOD") == "POST":
    post_data = get_post_data()
    username = post_data.get("username", [""])[0].strip()
    if username:
        new_sid = str(uuid.uuid4())
        save_session(new_sid, {
            "user": username,
            "visits": 1,
            "joined": datetime.now().strftime("%Y-%m-%d %H:%M")
        })
        print(f"Set-Cookie: {SESSION_COOKIE}={new_sid}; Path=/; HttpOnly")
        print("Status: 303 See Other")
        print("Location: app.py")
        print("\r\n")
        sys.exit(0)

# --- UI Generation ---

# Colors & Classes based on theme
# Using Tailwind's dark mode strategy manually or just class swapping.
# Let's simple swap wrapper classes.
is_dark = (theme == "dark")

# Base styles
bg_class = "bg-slate-900" if is_dark else "bg-gray-50"
text_class = "text-white" if is_dark else "text-slate-900"
card_class = "bg-slate-800/50" if is_dark else "bg-white/80"
input_class = "bg-slate-700/50 border-slate-600 text-white placeholder-slate-400" if is_dark else "bg-white border-gray-200 text-slate-900 placeholder-gray-400"
accent_text = "text-indigo-400" if is_dark else "text-indigo-600"
sub_text = "text-slate-400" if is_dark else "text-slate-500"

# Main Content Logic
if session:
    # Update visits
    session["visits"] += 1
    save_session(sid, session)
    
    view_content = f"""
    <div class="space-y-6">
        <div class="flex items-center space-x-4">
            <div class="h-12 w-12 rounded-full bg-indigo-500 flex items-center justify-center text-xl font-bold text-white shadow-lg ring-2 ring-indigo-400/30">
                {session['user'][0].upper()}
            </div>
            <div>
                <h2 class="text-2xl font-bold {text_class}">Hello, {session['user']}</h2>
                <p class="{sub_text} text-sm">Session active</p>
            </div>
        </div>

        <div class="grid grid-cols-2 gap-4">
            <div class="p-4 rounded-xl { 'bg-slate-700/30' if is_dark else 'bg-gray-100' } border border-transparent hover:border-indigo-500/30 transition-all duration-200">
                <div class="{accent_text} text-2xl font-bold font-mono">{session['visits']}</div>
                <div class="{sub_text} text-xs uppercase tracking-wider font-semibold mt-1">Total Visits</div>
            </div>
            <div class="p-4 rounded-xl { 'bg-slate-700/30' if is_dark else 'bg-gray-100' } border border-transparent hover:border-emerald-500/30 transition-all duration-200">
                <div class="text-emerald-500 text-2xl font-bold">ACTIVE</div>
                <div class="{sub_text} text-xs uppercase tracking-wider font-semibold mt-1">Status</div>
            </div>
        </div>

        <div class="pt-4 border-t { 'border-slate-700' if is_dark else 'border-gray-200' }">
             <div class="flex items-center justify-between mb-6">
                <span class="{sub_text} text-xs">Started: {session['joined']}</span>
             </div>
             
             <a href="?logout=1" 
               class="flex items-center justify-center w-full py-3 px-4 rounded-xl border border-red-500/30 text-red-500 hover:bg-red-500 hover:text-white transition-all duration-200 font-medium group">
               <span>Sign Out</span>
               <svg xmlns="http://www.w3.org/2000/svg" class="h-4 w-4 ml-2 transition-transform group-hover:translate-x-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 16l4-4m0 0l-4-4m4 4H7m6 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h4a3 3 0 013 3v1" />
                </svg>
             </a>
        </div>
    </div>
    """
else:
    view_content = f"""
    <div class="text-center space-y-2 mb-8">
        <h1 class="text-3xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-indigo-400 to-cyan-400">Welcome Back</h1>
        <p class="{sub_text}">Enter your credentials to access the secure dashboard.</p>
    </div>

    <form method="POST" class="space-y-5">
        <div class="space-y-2 text-left">
            <label class="text-sm font-medium {sub_text} ml-1">Username</label>
            <input type="text" name="username" placeholder="e.g. Neo" required autofocus
                   class="w-full px-4 py-3 rounded-xl border-2 focus:border-indigo-500 focus:ring-0 outline-none transition-all duration-200 {input_class}">
        </div>

        <button type="submit" 
                class="w-full py-3.5 px-4 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-600 hover:from-indigo-500 hover:to-violet-500 text-white font-semibold shadow-lg shadow-indigo-500/30 transform hover:-translate-y-0.5 transition-all duration-200">
            Enter Dashboard
        </button>
    </form>
    """

# Final HTML Assembly using Tailwind CDN
html_body = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Premium App</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <script>
        tailwind.config = {{
            theme: {{
                fontFamily: {{
                    sans: ['Outfit', 'sans-serif'],
                }},
                extend: {{
                    animation: {{
                        'fade-in-up': 'fadeInUp 0.3s ease-out forwards',
                    }},
                    keyframes: {{
                        fadeInUp: {{
                            '0%': {{ opacity: '0', transform: 'translateY(10px)' }},
                            '100%': {{ opacity: '1', transform: 'translateY(0)' }},
                        }}
                    }}
                }}
            }}
        }}
    </script>
</head>
<body class="{bg_class} min-h-screen flex items-center justify-center p-4 transition-colors duration-300">

    <!-- Card Container -->
    <div class="relative w-full max-w-md">
        <!-- Blur effect behind -->
        <div class="absolute -top-4 -left-4 w-72 h-72 bg-purple-500 rounded-full mix-blend-multiply filter blur-xl opacity-20 animate-blob"></div>
        <div class="absolute -bottom-4 -right-4 w-72 h-72 bg-yellow-500 rounded-full mix-blend-multiply filter blur-xl opacity-20 animate-blob animation-delay-2000"></div>
        <div class="absolute -bottom-8 left-20 w-72 h-72 bg-indigo-500 rounded-full mix-blend-multiply filter blur-xl opacity-20 animate-blob animation-delay-4000"></div>

        <!-- Main Card -->
        <div class="{card_class} backdrop-blur-xl rounded-2xl shadow-2xl border border-white/10 p-8 relative overflow-hidden animate-fade-in-up">
            
            <!-- Theme Toggle -->
            <a href="?toggle_theme=1" class="absolute top-4 right-4 p-2 rounded-full hover:bg-black/5 dark:hover:bg-white/10 transition-colors duration-200 group" title="Toggle Theme">
                <span class="text-xl group-hover:scale-110 block transition-transform duration-200">
                    { '☀️' if is_dark else '🌙' }
                </span>
            </a>

            {view_content}

        </div>
    </div>

</body>
</html>
"""

print("Content-Type: text/html")
print(f"Content-Length: {len(html_body.encode('utf-8'))}")
print("\r\n")
print(html_body)
