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

std::string HttpResponse::serialize() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << getStatusText(statusCode) << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
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
         << "<html><head><title>Error " << code << "</title>"
         << "<style>body{font-family:Arial,sans-serif;margin:40px;line-height:1.6}\n"
         << "h1{color:#D33;} .container{max-width:600px;margin:0 auto;}</style></head>"
         << "<body><div class='container'><h1>Error " << code << "</h1>"
         << "<p>" << getStatusText(code) << ": ";
    
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
