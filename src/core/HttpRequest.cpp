/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:24:58 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:24:58 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cerrno>
#include <cstring>

HttpRequest::HttpRequest() : contentLength(0) {}

HttpRequest::~HttpRequest() {}

bool HttpRequest::parse(const std::string& request, std::vector<char>& rawbody) {
    rawBody = rawbody;
    return parseHeaders(request);
}

bool HttpRequest::parseHeaders(const std::string& headers) {
    std::istringstream stream(headers);
    std::string line;

    if (!std::getline(stream, line)) {
        // std::cerr << "Failed to read request line" << std::endl;
        return false;
    }
    if (line[line.length() - 1] == '\r') {
        line.erase(line.length() - 1);
    }
    if (!parseRequestLine(line)) {
        // std::cerr << "Failed to parse request line: [" << line << "]" << std::endl;
        return false;
    }

    // Parse headers
    while (std::getline(stream, line)) {
        if (line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }
        if (line.empty()) {
            break;
        }
        if (!parseHeader(line)) {
            std::cerr << "Failed to parse header: [" << line << "]" << std::endl;
            return false;
        }
    }

    // Get content length
    std::map<std::string, std::string>::iterator it = this->headers.find("Content-Length");
    if (it != this->headers.end()) {
        contentLength = std::atoi(it->second.c_str());
    }

    std::map<std::string, std::string>::iterator itc = this->headers.find("Content-Type");
    if (itc != this->headers.end()) {
        contenttype = itc->second;
    }

    std::map<std::string, std::string>::iterator its = this->headers.find("Cookie");
    if (its != this->headers.end()) {
        cookie = its->second;
    }

    return true;
}

bool HttpRequest::parse(int clientSocket) {
    char buffer[4096];
    std::string request;
    ssize_t bytesRead;
    size_t totalRead = 0;
    bool headersComplete = false;
    size_t expectedContentLength = 0;
    
    // Read headers first
    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        request.append(buffer, bytesRead);
        
        // Check if we've received all headers
        size_t headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            // Parse headers
            std::string headers = request.substr(0, headerEnd);
            if (!parseHeaders(headers)) {
                std::cerr << "Failed to parse headers" << std::endl;
                return false;
            }
            
            // Get content length
            std::map<std::string, std::string>::iterator it = this->headers.find("Content-Length");
            if (it != this->headers.end()) {
                expectedContentLength = std::atoi(it->second.c_str());
                std::cout << "Expected content length: " << expectedContentLength << std::endl;
            }
            
            // Move remaining data to body
            body = request.substr(headerEnd + 4);
            totalRead = body.length();
            std::cout << "Initial body read: " << totalRead << " bytes" << std::endl;
            
            // If we have all the content, we're done
            if (expectedContentLength == 0 || totalRead >= expectedContentLength) {
                std::cout << "All content received" << std::endl;
                return validateRequest();
            }
            
            headersComplete = true;
            break;
        }
    }
    
    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "Failed to read from socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // If we have headers and need to read more body
    if (headersComplete && expectedContentLength > 0) {
        std::cout << "Reading remaining body data..." << std::endl;
        while (totalRead < expectedContentLength) {
            bytesRead = read(clientSocket, buffer, std::min(sizeof(buffer) - 1, expectedContentLength - totalRead));
            if (bytesRead <= 0) {
                if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    // Would block, try again
                    continue;
                }
                std::cerr << "Failed to read request body: " << strerror(errno) << std::endl;
                return false;
            }
            body.append(buffer, bytesRead);
            totalRead += bytesRead;
            std::cout << "Read " << totalRead << " of " << expectedContentLength << " bytes" << std::endl;
        }
    }
    
    std::cout << "Request parsing complete" << std::endl;
    return validateRequest();
}

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream lineStream(line);
    
    if (!(lineStream >> method >> path >> version)) {
        return false;
    }
    if (version.find("HTTP") == std::string::npos){
        return false;
    }
    // Extract query string if present
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        query = path.substr(queryPos + 1);
        path = path.substr(0, queryPos);
    }
    
    return true;
}

bool HttpRequest::parseHeader(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }
    
    std::string name = line.substr(0, colonPos);
    std::string value = line.substr(colonPos + 1);
    
    // Trim leading/trailing whitespace
    while (!value.empty() && std::isspace(value[0])) {
        value.erase(0, 1);
    }
    while (!value.empty() && std::isspace(value[value.length() - 1])) {
        value.erase(value.length() - 1);
    }
    
    headers[name] = value;
    return true;
}

bool HttpRequest::validateRequest() {
    if (method.empty() || path.empty() || version.empty()) {
        return false;
    }
    
    if (version != "HTTP/1.0" && version != "HTTP/1.1") {
        return false;
    }
    
    return true;
} 
