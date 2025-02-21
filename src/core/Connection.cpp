/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:23:36 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:23:36 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "FileHandler.hpp"
#include "Router.hpp"
#include <iostream>

Connection::Connection()
    : socket(-1), config(NULL), lastActivity(time(NULL)), keepAlive(false) {}

Connection::Connection(int socket, const ServerConfig& config)
    : socket(socket), config(&config), lastActivity(time(NULL)), keepAlive(false) {}

Connection::Connection(const Connection& other)
    : socket(other.socket), config(other.config), requestBuffer(other.requestBuffer),
      responseBuffer(other.responseBuffer), lastActivity(other.lastActivity),
      keepAlive(other.keepAlive) {}

Connection& Connection::operator=(const Connection& other) {
    if (this != &other) {
        socket = other.socket;
        config = other.config;
        requestBuffer = other.requestBuffer;
        responseBuffer = other.responseBuffer;
        lastActivity = other.lastActivity;
        keepAlive = other.keepAlive;
    }
    return *this;
}

int Connection::getSocket() const {
    return socket;
}

time_t Connection::getLastActivityTime() const {
    return lastActivity;
}

bool Connection::hasDataToWrite() const {
    return !responseBuffer.empty();
}

const std::string& Connection::getResponse() const {
    return responseBuffer;
}

void Connection::clearResponse(size_t bytes) {
    responseBuffer = responseBuffer.substr(bytes);
    updateLastActivity();
}

bool Connection::appendRequestData(const std::string& data, int socket) {
    requestBuffer += data;
    std::istringstream  stream(requestBuffer);
    std::string         line;

    if (!std::getline(stream, line)) {
        std::cerr << "Failed to read request line" << std::endl;
        return false;
    }
    if (line.find("POST") == std::string::npos)
    {
        return (true);
    }

    std::size_t header_end = requestBuffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }
    int content_length = -1;
    
    while (std::getline(stream, line) && line != "\r") {
        if (line.find("Content-Length:") == 0) {
            std::istringstream len_stream(line.substr(15));
            if (!(len_stream >> content_length)) {
                std::cerr << "Invalid Content-Length value" << std::endl;
                return false;
            }
        }
    }
    std::cout << "\n" << content_length << "content_length\n";
    if (content_length == -1) {
        std::cerr << "Missing Content-Length header" << std::endl;
        return false;
    }

    std::size_t body_start = header_end + 4; 
    std::size_t received_body_size = requestBuffer.size() - body_start;
    if (received_body_size < static_cast<size_t>(content_length)) {
        std::cout << "Waiting for more body data...\n";
        return false;
    }
    std::string body = requestBuffer.substr(body_start, content_length);
    std::cout << "Received POST body:\n" << body << std::endl;
    return true;
}


void Connection::processRequest() {
    size_t pos;
    std::string raw = requestBuffer;
    while ((pos = requestBuffer.find("\r\n\r\n")) != std::string::npos) {
        std::string request = requestBuffer.substr(0, pos + 4);
        std::cout << "\n\nrequest---\n\n" << requestBuffer << "\n\n";
        requestBuffer = requestBuffer.substr(pos + 4);
        HttpRequest httpRequest;
        if (httpRequest.parse(request)) {
            std::cout << "\nProcessing request: " << httpRequest.getMethod() << " " << httpRequest.getPath() << std::endl;
            
            HttpResponse response;
            const RouteConfig* route = Router::findRoute(*config, httpRequest.getPath());
            
            if (route) {
                std::cout << "Found route with path: " << route->path << std::endl;
                std::cout << "Route upload store: " << route->uploadStore << std::endl;
                
                try {
                    if (httpRequest.getContentLength() > route->clientMaxBodySize) {
                        std::cerr << "Content length " << httpRequest.getContentLength() 
                                << " exceeds max body size " << route->clientMaxBodySize << std::endl;
                        response.setStatus(413);
                        response.setBody(HttpResponse::getDefaultErrorPage(413));
                    } else if (httpRequest.getMethod() == "GET") {
                        if (httpRequest.getPath() == route->path)
                        {
                            FileHandler::handleGet(route->root + "/" + route->index, response, route->autoIndex);
                        }
                        else
                        {
                            FileHandler::handleGet(route->root + httpRequest.getPath(), response, route->autoIndex);
                        }
                    } else if (httpRequest.getMethod() == "POST") {

                        FILE* pipe = popen("python3 ./upload.py", "w");
                        if (!pipe) {
                            std::cerr << "Failed to open pipe to Python script\n";
                        }
                        fwrite(raw.c_str(), 1, raw.size(), pipe);
                        pclose(pipe);

                        std::string uploadPath;
                        if (route->uploadStore.empty()) {
                            uploadPath = route->root + httpRequest.getPath();
                            std::cout << "Using root path for upload: " << uploadPath << std::endl;
                        } else {
                            std::string filename = httpRequest.getPath();
                            if (filename.find(route->path) == 0) {
                                filename = filename.substr(route->path.length());
                            }
                            if (!filename.empty() && filename[0] == '/') {
                                filename = filename.substr(1);
                            }
                            if (!FileHandler::createDirectories(route->uploadStore)) {
                                std::cerr << "Failed to create upload directory: " << route->uploadStore << std::endl;
                                response.setStatus(500);
                                response.setBody(HttpResponse::getDefaultErrorPage(500));
                                return;
                            }
                            uploadPath = route->uploadStore + "/" + filename;
                            std::cout << "Using upload store path: " << uploadPath << std::endl;
                        }
                        std::cout << "Request body size: " << httpRequest.getBody().length() << std::endl;
                        FileHandler::handlePost(uploadPath, requestBuffer, response, "file");
                    } else if (httpRequest.getMethod() == "DELETE") {
                        std::string deletePath;
                        if (route->uploadStore.empty()) {
                            deletePath = route->root + httpRequest.getPath();
                        } else {
                            std::string filename = httpRequest.getPath();
                            if (filename.find(route->path) == 0) {
                                filename = filename.substr(route->path.length());
                            }
                            if (!filename.empty() && filename[0] == '/') {
                                filename = filename.substr(1);
                            }
                            deletePath = route->uploadStore + "/" + filename;
                        }
                        std::cout << "Using delete path: " << deletePath << std::endl;
                        FileHandler::handleDelete(deletePath, response);
                    } else {
                        response.setStatus(405);
                        response.setBody(HttpResponse::getDefaultErrorPage(405));
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing request: " << e.what() << std::endl;
                    response.setStatus(500);
                    response.setBody(HttpResponse::getDefaultErrorPage(500));
                }
            } else {
                std::cerr << "No matching route found for path: " << httpRequest.getPath() << std::endl;
                response.setStatus(404);
                response.setBody(HttpResponse::getDefaultErrorPage(404));
            }
            
            responseBuffer += response.serialize();
        } else {
            std::cerr << "Failed to parse request" << std::endl;
            HttpResponse errorResponse;
            errorResponse.setStatus(400);
            errorResponse.setBody(HttpResponse::getDefaultErrorPage(400));
            responseBuffer += errorResponse.serialize();
            break;
        }
    }
}

void Connection::updateLastActivity() {
    lastActivity = time(NULL);
} 