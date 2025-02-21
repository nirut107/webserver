#!/usr/bin/env python3
import os
import sys

def print_env_vars():
    print("Content-Type: text/html\r\n\r\n")
    print("<!DOCTYPE html>")
    print("<html><head><title>CGI Test</title></head>")
    print("<body>")
    print("<h1>CGI Environment Variables</h1>")
    print("<table border='1'>")
    for key, value in sorted(os.environ.items()):
        print(f"<tr><td>{key}</td><td>{value}</td></tr>")
    print("</table>")
    
    if os.environ.get('REQUEST_METHOD') == 'POST':
        print("<h2>POST Data</h2>")
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        post_data = sys.stdin.read(content_length)
        print(f"<pre>{post_data}</pre>")
    
    print("</body></html>") 