import sys
import re

def extract_filename(data: str) -> str:
    """
    Extracts the filename from the Content-Disposition header.
    """
    match = re.search(r'Content-Disposition:.*filename="([^"]+)"', data)
    return match.group(1) if match else None

def trim_boundary(data: str) -> str:
    """
    Removes HTTP headers and multipart boundary from form-data input and extracts the actual content.
    """
    # Remove HTTP headers
    parts = data.split("\n\n", 1)
    body = parts[1] if len(parts) > 1 else data
    
    lines = body.strip().split("\n")
    
    # Identify boundary
    boundary = None
    for line in lines:
        if line.startswith("--"):
            boundary = line.strip()
            break
    
    if boundary:
        # Extract content between boundary markers
        sections = body.split(boundary)
        for section in sections:
            if "Content-Disposition" in section and "Content-Type" in section:
                content = section.split("\n\n", 1)[-1]  # Extract script after headers
                content_lines = content.split("\n")
                filtered_content = "\n".join(line for line in content_lines if not line.startswith("Content-Disposition") and not line.startswith("Content-Type"))
                return filtered_content.strip()
    
    return body.strip()

# Path where you want to save the file
upload_dir = "./www/uploads"  # Use a directory where you have write access

# Ensure the directory exists
import os
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir, 0o777)

# Read raw POST data from stdin
post_data = sys.stdin.read()
print("\n\n\npost_data\n", post_data, "\npython\n")

# Check if data was received
if post_data:
    # Specify the file path and name
    file_name = extract_filename(post_data)
    file_path = os.path.join(upload_dir, file_name)
    trimmed_data = trim_boundary(post_data)
    print("\n\n\ntrim_data------\n", trimmed_data)
    # Write the text to the file
    try:
        with open(file_path, "w") as file:
            file.write(trimmed_data)
        print(f"Text data successfully written to file: {file_name}")
    except Exception as e:
        print(f"Failed to write text to file: {str(e)}")
else:
    print("No text data received.")
