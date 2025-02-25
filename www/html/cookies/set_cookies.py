#!/usr/bin/env python3

import cgi
import http.cookies

def set_cookies():
    
    print("Content-Type: text/html")

    # Get form input
    form = cgi.FieldStorage()
    username = form.getvalue("username", "Guest")  # Default to "Guest" if empty

    # Set the cookie
    cookie = http.cookies.SimpleCookie()
    cookie["username"] = username
    cookie["username"]["path"] = "/"  # Applies to the whole site
    cookie["username"]["max-age"] = 3600  # Expires in 1 hour

    # Print the cookie header
    print(cookie.output())  
    print("\n")  # Required newline after headers

    # HTML Response
    print(f"""
    <html>
    <body>
        <h1>Hello, {username}! Your cookie has been set.</h1>
        <a href="get_cookie.py">Go to Welcome Page</a>
    </body>
    </html>
    """)


if __name__ == "__main__":
    set_cookies()