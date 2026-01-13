#!/usr/bin/env python3

import os
import sqlite3


def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    session_id = None

    if "sessionID=" in cookie_header:
        session_id = cookie_header.split("sessionID=")[1].split(";")[0]

    try:
        conn = sqlite3.connect("sessions.db")
        cursor = conn.cursor()
        cursor.execute(
            "SELECT full_name FROM sessions WHERE session_id=?", (session_id,)
        )
        user = cursor.fetchone()
        conn.close()
    except:
        print("Status: 500 Internal Server Error")
        return

    message = ""
    if user:
        message = f"<h1>Welcome back, {user[0]}!</h1>"
        print("Status: 200 OK")
    else:
        message = "h1>Session expired or invalid. Please login.</h1>"
        print("Status: 303 See Other")
        print("Location: /login.html")
    print("Content-Type: text/html")
    print(f"Content-Length: {len(message)}")
    print()
    print(message)


if __name__ == "__main__":
    main()
