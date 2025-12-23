#!/usr/bin/env python3

import sys
import os
import secrets
import sqlite3

def getFormatedHeaders(headers):
    lines = [f"{key}: {value}" for key, value in self._headers.items()]
    return "\r\n".join(lines) + "\r\n"

def generate_session_id():
    return secrets.token_urlsafe(42)

def extrect_session_id(cookie=os.environ.get("HTTP_COOKIE")):
    if not cookie:
        return None
    id = None
    for x in cookie.split(";"):
        entry = x.strip().split("=")
        if len(entry) > 1 and entry[0] == "sessionId":
            return entry[1]
    return None

class DB:
    def __init__(self):
        self.conn = sqlite3.connect('clients.db')
        self.cursor = conn.cursor()
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS user_sessions (
                session_id TEXT PRIMARY KEY,
                first_name TEXT NOT NULL,
                last_name TEXT NOT NULL
            )
        ''')
        self.conn.commit()

    def addNewSession(self, first_name, last_name):
        session_id = generate_session_id()
        try:
            cursor.execute('''
                INSERT INTO user_sessions (session_id, first_name, last_name)
                VALUES (?, ?, ?)
            ''', (session_id, first_name, last_name))
            conn.commit()
            return session_id
        except sqlite3.Error as e:
            print(f"An error occurred: {e}")
            return None


db = DB()
headers = dict()
sessionId = extrect_session_id()
if not sessionId:
    sessionId = generate_session_id()
    headers["Set-Cookie"] = f"sessionId={sessionId}"

headers.add("Content-Type", "text/plain")
headers.add("Content-Length", "5")
headers.add("Status", "202")
