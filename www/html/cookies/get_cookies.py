#!/usr/bin/env python3

import os
import http.cookies

print("Content-Type: text/html\n")  # Required header

# Get the stored cookie
cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
username = cookie.get("username")

# HTML Response
print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cookie & Session Test</title>
    <link rel="stylesheet" href="styles.css">
</head>
<body>
      <h1>🍪 Cookie & Session Test</h1>""")
if username:
    print(f"<h1>Welcome back, {username.value}!</h1>")
else:
    print("""<div class="section">
            <h2>Enter Your Name</h2>
            <form action="get_cookies.py" method="GET">
                <input type="text" name="username" placeholder="Enter your name">
                <button class="button" type="submit">Set Cookie</button>
            </form>
            
        </div>""")
print("</body></html>")
