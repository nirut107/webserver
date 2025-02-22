#!/usr/bin/env python3

import sys
import re
import os


def process_uploaded_bin():

    post_data = sys.argv[1]

    pattern = r'[?&]x=([^&\s]+)'

    # Search for the pattern in the request string
    match = re.search(pattern, post_data)
    x_value = match.group(1)

    print(f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>42 Webserv Space Station</title>
        <style>
            * {{
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }}
            body {{
                font-family: 'Arial', sans-serif;
                background: linear-gradient(to bottom, #0B1026 0%, #1B2735 50%, #090A0F 100%);
                color: #fff;
                min-height: 100vh;
                padding: 20px;
                position: relative;
                overflow-x: hidden;
            }}
            body::before {{
                content: '';
                position: absolute;
                width: 100%;
                height: 100%;
                top: 0;
                left: 0;
                background-image: 
                    radial-gradient(2px 2px at 20px 30px, #ffffff, rgba(0,0,0,0)),
                    radial-gradient(2px 2px at 40px 70px, #ffffff, rgba(0,0,0,0)),
                    radial-gradient(2px 2px at 50px 160px, #ffffff, rgba(0,0,0,0)),
                    radial-gradient(2px 2px at 90px 40px, #ffffff, rgba(0,0,0,0)),
                    radial-gradient(2px 2px at 130px 80px, #ffffff, rgba(0,0,0,0));
                background-repeat: repeat;
                animation: twinkle 5s infinite linear;
                z-index: -1;
            }}
            @keyframes twinkle {{
                from {{ transform: translateY(0); }}
                to {{ transform: translateY(-100px); }}
            }}
            .container {{
                max-width: 800px;
                margin: 0 auto;
                position: relative;
            }}
            h1 {{
                text-align: center;
                color: #7EB6FF;
                font-size: 2.5em;
                margin-bottom: 40px;
                text-shadow: 0 0 10px rgba(126, 182, 255, 0.5);
            }}
            .section {{
                background: rgba(255, 255, 255, 0.1);
                backdrop-filter: blur(10px);
                border: 1px solid rgba(255, 255, 255, 0.2);
                padding: 25px;
                margin: 30px 0;
                border-radius: 15px;
                box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
                transition: transform 0.3s ease;
            }}
            .section:hover {{
                transform: translateY(-5px);
            }}
            h2 {{
                color: #7EB6FF;
                margin-bottom: 20px;
                font-size: 1.5em;
            }}
            .button {{
                background: linear-gradient(45deg, #3498db, #2980b9);
                color: white;
                padding: 12px 24px;
                border: none;
                border-radius: 25px;
                cursor: pointer;
                margin: 5px;
                transition: all 0.3s ease;
                text-transform: uppercase;
                letter-spacing: 1px;
                font-weight: bold;
            }}
            .button:hover {{
                background: linear-gradient(45deg, #2980b9, #3498db);
                transform: scale(1.05);
                box-shadow: 0 0 15px rgba(52, 152, 219, 0.5);
            }}
            .delete-button {{
                background: linear-gradient(45deg, #e74c3c, #c0392b);
            }}
            .delete-button:hover {{
                background: linear-gradient(45deg, #c0392b, #e74c3c);
            }}
            input[type="text"], input[type="file"] {{
                background: rgba(255, 255, 255, 0.1);
                border: 1px solid rgba(255, 255, 255, 0.2);
                padding: 10px 15px;
                border-radius: 25px;
                color: white;
                width: 100%;
                margin-bottom: 15px;
            }}
            input[type="file"] {{
                padding: 10px;
            }}
            #response, #getResponse, #uploadResponse, #deleteResponse {{
                background: rgba(0, 0, 0, 0.3);
                padding: 15px;
                margin-top: 15px;
                border-radius: 10px;
                border: 1px solid rgba(255, 255, 255, 0.1);
            }}
            a {{
                color: #7EB6FF;
                text-decoration: none;
                transition: all 0.3s ease;
            }}
            a:hover {{
                color: #ffffff;
                text-shadow: 0 0 10px rgba(126, 182, 255, 0.5);
            }}
            .astronaut {{
                position: fixed;
                bottom: 20px;
                right: 20px;
                width: 100px;
                height: 100px;
                background: url('/api/placeholder/100/100') no-repeat center center;
                animation: float 6s ease-in-out infinite;
            }}
            @keyframes float {{
                0% {{ transform: translateY(0px) rotate(0deg); }}
                50% {{ transform: translateY(-20px) rotate(5deg); }}
                100% {{ transform: translateY(0px) rotate(0deg); }}
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>HELLO {x_value if x_value else "No name provided"}</h1>
            <div class="section">
                <h2>🛸 CGI Test</h2>
                <form id="uploadForm" method="get" action="/cgi-bin/setName.py">
                    <input type="text" placeholder="input your name" name="x">
                    <button type="submit" class="button">Say Hi</button>
                </form>
                <div id="uploadResponse"></div>
                
            </div>
            <a href="/index.html">Go back to Home</a>
        </div>
        <script></script>
    </body>
    </html>
    """)

if __name__ == "__main__":
    process_uploaded_bin()
