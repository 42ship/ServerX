#!/usr/bin/env python3

import json
import os
import sqlite3
import sys
import uuid


def init_db():
    conn = sqlite3.connect("sessions.db")
    cursor = conn.cursor()
    cursor.execute("""CREATE TABLE IF NOT EXISTS sessions
                      (session_id TEXT PRIMARY KEY, full_name TEXT, age INTEGER)""")
    conn.commit()
    return conn


def main():
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        raw_body = sys.stdin.read(content_length)
        data = json.loads(raw_body)
    except (ValueError, json.JSONDecodeError):
        print("Status: 400 Bad Request\r\n\r\n")
        return

    full_name = data.get("fullName", "Unknown")
    age = data.get("age", 0)

    session_id = str(uuid.uuid4())

    conn = init_db()
    cursor = conn.cursor()
    cursor.execute(
        "INSERT INTO sessions (session_id, full_name, age) VALUES (?, ?, ?)",
        (session_id, full_name, age),
    )
    conn.commit()
    conn.close()

    print(f"Set-Cookie: sessionID={session_id}; HttpOnly; Path=/")
    print("Content-Type: application/json")
    print("Status: 200 OK")
    print("\r\n")
    print(json.dumps({"message": "Login successful", "sessionID": session_id}))


if __name__ == "__main__":
    main()
