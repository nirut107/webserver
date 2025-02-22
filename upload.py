import os
import sys

def parse_multipart_data(bin_data, boundary):
    """ Function to parse multipart data from binary """
    parts = bin_data.split(f'--{boundary}'.encode())
    files = []

    for part in parts:
        part = part.strip()
        if not part or part == b"--":
            continue
        
        if b'\r\n\r\n' in part:
            headers, body = part.split(b'\r\n\r\n', 1)

            if b'Content-Disposition' in headers and b'filename=' in headers:
                content_disposition = [line for line in headers.split(b'\r\n') if b"Content-Disposition" in line][0]
                filename = content_disposition.split(b'filename="')[1].split(b'"')[0].decode('utf-8')
                files.append((filename, body))
    
    return files

def process_uploaded_bin(file_path):
    
    upload_path = sys.argv[1]
    """ Function to read uploaded.bin and recreate uploaded files """
    
    with open(file_path, 'rb') as f:
        bin_data = f.read()

    
    boundary_start = bin_data.find(b'boundary=')
    if boundary_start == -1:
        print("Error: Boundary not found")
        return
    
    boundary = bin_data[boundary_start + 9:].split(b'\r\n')[0].decode('utf-8')
    
    files = parse_multipart_data(bin_data, boundary)
    
    upload_dir = os.path.join(os.path.dirname(__file__), upload_path)
    os.makedirs(upload_dir, exist_ok=True)

    for filename, file_content in files:
        file_paths = os.path.join(upload_dir, filename)
        
        with open(file_paths, 'wb') as f:
            f.write(file_content)
        print(f"File {filename} successfully saved to {file_paths}")
    
    os.remove(file_path)

if __name__ == "__main__":
    process_uploaded_bin("uploaded.bin")
