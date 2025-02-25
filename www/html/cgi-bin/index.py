#!/usr/bin/env python3

import os
import sys

def parse_multipart_formdata(content_type, content_length):
    """ Read and process multipart form-data input from stdin """
    
    body = sys.stdin.read(int(content_length))
    
    boundary = content_type.split("boundary=")[-1]
    
    parts = body.split("--" + boundary)
    
    for part in parts:
        if "Content-Disposition" in part and "filename=" in part:
            headers, file_data = part.split("\r\n\r\n", 1)
            
            filename = headers.split("filename=")[-1].split('"')[1]
            
            file_data = file_data.rsplit("\r\n", 1)[0]
            

            with open(f"/tmp/{filename}", "wb") as f:
                f.write(file_data.encode())

            return filename

    return None

content_type = os.getenv("CONTENT_TYPE", "")
content_length = os.getenv("CONTENT_LENGTH", "0")

filename = parse_multipart_formdata(content_type, content_length)