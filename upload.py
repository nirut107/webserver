import sys

# Path where you want to save the file
upload_dir = "/home/nsomrod/Downloads/non_webserv/www"  # Use a directory where you have write access

# Ensure the directory exists
import os
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir, 0o777)

# Read raw POST data from stdin
post_data = sys.stdin.read()
print("\n\n\n", post_data, "\npython\n")

# Check if data was received
if post_data:
    # Specify the file path and name
    file_name = "uploaded_text.txt"
    file_path = os.path.join(upload_dir, file_name)

    # Write the text to the file
    try:
        with open(file_path, "w") as file:
            file.write(post_data)
        print(f"Text data successfully written to file: {file_name}")
    except Exception as e:
        print(f"Failed to write text to file: {str(e)}")
else:
    print("No text data received.")
