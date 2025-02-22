#!/usr/bin/env python3
import cgi
import os

# Set the content-type for the HTTP response
print("Content-type: text/html\n")
print("python")

# Parse the incoming POST data
form = cgi.FieldStorage()
print(form)

# Check if the form contains a file
if "file" in form:
    # Get the uploaded file
    uploaded_file = form["file"]

    # Ensure the file is a valid field (may be None if no file is uploaded)
    if uploaded_file.filename:
        # Get the filename and content
        filename = uploaded_file.filename
        file_content = uploaded_file.file.read()

        # Define the path where the file will be saved
        save_path = f"www/http/uploads/{filename}"

        # Ensure the upload directory exists
        os.makedirs(os.path.dirname(save_path), exist_ok=True)

        # Write the file to the specified path
        with open(save_path, "wb") as f:
            f.write(file_content)
        
        # Inform the user the upload was successful
        print(f"<h2>File uploaded successfully!</h2>")
        print(f"<p>File saved as: {filename}</p>")

    else:
        print("<h2>No file uploaded or file is invalid.</h2>")
else:
    print("<h2>No file found in the request.</h2>")
