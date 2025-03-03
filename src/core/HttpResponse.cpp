/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:25:07 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:25:07 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>
#include <unistd.h>

HttpResponse::HttpResponse() : statusCode(200) {
    headers["Server"] = "Webserv/1.0";
    headers["Connection"] = "close";
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatus(int code) {
    statusCode = code;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void HttpResponse::setBody(const std::string& content) {
    body = content;
    std::ostringstream contentLength;
    contentLength << body.length();
    headers["Content-Length"] = contentLength.str();
}

bool HttpResponse::send(int clientSocket) {
    std::string response = serialize();
    size_t totalSent = 0;
    
    while (totalSent < response.length()) {
        ssize_t sent = write(clientSocket, response.c_str() + totalSent, response.length() - totalSent);
        if (sent <= 0) {
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}


static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start])) {
        ++start;
    }

    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1])) {
        --end;
    }

    return str.substr(start, end - start);
}


std::string HttpResponse::serialize() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << getStatusText(statusCode) << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << trim(it->first) << ": " << it->second << "\r\n";
    }
    
    response << "\r\n" << body;
    return response.str();
}

std::string HttpResponse::getStatusText(int code) {
    switch (code) {
        case 200: return "";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Request Entity Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

std::string HttpResponse::getDefaultErrorPage(int code) {
    std::ostringstream html;
    
    html << "<!DOCTYPE html>\n"
              << "<html lang='en'>\n"
              << "<head>\n"
              << "    <meta charset='UTF-8'>\n"
              << "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
              << "    <title>Error " << code << "</title>\n"
              << "    <style>\n"
              << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
              << "        .containerz { text-align: center; font-family: 'Arial', sans-serif; background: linear-gradient(to bottom, #0B1026 0%, #1B2735 50%, #090A0F 100%); color: #fff;  padding: 20px; position: relative; overflow-x: hidden; }\n"
              << "        h1 { text-align: center; color: #7EB6FF; font-size: 2.5em; margin-bottom: 40px; text-shadow: 0 0 10px rgba(126, 182, 255, 0.5); }\n"
              << "        .section { background: rgba(255, 255, 255, 0.1); backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.2); padding: 25px; margin: 30px 0; border-radius: 15px; box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37); }\n"
              << "        .button:hover { background: linear-gradient(45deg, #2980b9, #3498db); transform: scale(1.05); box-shadow: 0 0 15px rgba(52, 152, 219, 0.5); }\n"
              << "    </style>\n"
              << "</head>\n"
              << "<body>\n"
              << "    <div class='containerz'>\n"
              << "        <h1>Error " << code << "</h1>\n"
              << "        <div>\n"
              << "            <h2>" << getStatusText(code)<< "</h2>\n"
              << "        </div>\n";
    
    switch (code) {
        case 400: html << "The server cannot process the request due to client error."; break;
        case 401: html << "Authentication is required to access this resource."; break;
        case 403: html << "You do not have permission to access this resource."; break;
        case 404: html << "The requested resource could not be found on this server."; break;
        case 405: html << "The requested method is not allowed for this resource."; break;
        case 413: html << "The request entity is larger than the server is willing or able to process."; break;
        case 500: html << "The server encountered an internal error."; break;
        case 501: html << "The server does not support the functionality required."; break;
        case 502: html << "The server received an invalid response from the upstream server."; break;
        case 503: html << "The server is temporarily unavailable."; break;
        default: html << "An unexpected error occurred.";
    }
    
    html << "</p><p>If you believe this is a server error, please contact the administrator.</p>"
         << "</div></body></html>";
    
    return html.str();
}

void HttpResponse::clear() {
    statusCode = 200;
    headers.clear();
    body.clear();
} 

std::string HttpResponse::getHeader(const std::string& name) const
{
    std::string  ret; 
    try {
        ret = headers.at(name);
    } catch(std::exception &e)
    {
        ret = ""; 
    }
    return (ret);
}
