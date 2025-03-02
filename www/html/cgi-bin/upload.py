
import cgi
import os
import sys

boundary = os.environ.get('HTTP_BOUNDARY')
content_type = os.environ.get('CONTENT_TYPE')
upload_dir = os.environ.get('UPLOAD_DIR')

print(sys.stdin)
form = cgi.FieldStorage(fp=sys.stdin)

if "file" in form:
    file_item = form["file"]
    filename = file_item.filename
    filepath = os.path.join(upload_dir, filename)
    with open(filepath, "wb") as f:
        f.write(file_item.file.read())
    print("Content-Type: text/html\n")
    print("<html><body><h2>File uploaded successfully.</h2></body></html>")
else:
    print("Content-Type: text/html\n")
    print("<html><body><h2>No file uploaded.</h2></body></html>")
