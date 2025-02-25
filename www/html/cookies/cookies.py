#!/usr/bin/env python3

import cgi
import sys
import http.cookies

def set_cookies():
      
	form = cgi.FieldStorage()

	username = form.getvalue("username", "anonymous")

	cookie = http.cookies.SimpleCookie()
	cookie["username"] = username
	cookie["username"]["path"] = "/"
	cookie["username"]["max-age"] = 3600
    
	print(cookie.output())  
	print("\n")

	print("<html><body>")
	print(f"<h1>Hello, {username}! Your cookie has been set.</h1>")
	print("</body></html>")

	# print(f"""Welcome back, { username }""")
	# sys.exit()


if __name__ == "__main__":
    set_cookies()
