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
        return true;
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
    std::string boundary;

    while (std::getline(stream, line) && line != "\r") {
        if (line.find("Content-Length:") == 0) {
            std::istringstream len_stream(line.substr(15));
            if (!(len_stream >> content_length)) {
                std::cerr << "Invalid Content-Length value" << std::endl;
                return false;
            }
        }
        if (line.find("Content-Type:") == 0) {
            size_t boundary_pos = line.find("boundary=");
            if (boundary_pos != std::string::npos) {
                boundary = line.substr(boundary_pos + 9);
            }
        }
    }

    if (content_length == -1) {
        std::cerr << "Missing Content-Length header" << std::endl;
        return true;
    }

    char buffer[4000000];
    ssize_t bytesRead;
    size_t received_body_size = data.size() - header_end + 4;
    
    while (received_body_size < content_length) {
        bytesRead = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            requestBodyBin.insert(requestBodyBin.end(), buffer, buffer + bytesRead);
            received_body_size += bytesRead;
        }
    }

    std::cout << "check content_length :: received_body_size " << content_length << " :: " << received_body_size << "\n";

    if (!boundary.empty() && !requestBodyBin.empty()) {
        std::vector<char>::iterator it = requestBodyBin.begin();
        std::vector<char>::iterator end = requestBodyBin.end();

        while (it != end - boundary.size()) {
            if (std::equal(boundary.begin(), boundary.end(), it)) {
                break;
            }
            ++it;
        }
        if (it == end) {
            std::cerr << "Boundary not found" << std::endl;
            return false;
        }

        it += boundary.size();
        while (it != end && (*it == '\r' || *it == '\n')) {
            ++it;
        }

        while (it != end - 20) { 
            if (std::equal(it, it + 19, "Content-Disposition:")) {
                std::vector<char>::iterator header_start = it;
                while (header_start != end && *header_start != '\n') {
                    ++header_start;
                }

                std::string header(it, header_start);

                size_t filename_pos = header.find("filename=\"");
                if (filename_pos != std::string::npos) {
                    filename_pos += 10; // Move past "filename=\""
                    size_t filename_end = header.find("\"", filename_pos);
                    if (filename_end != std::string::npos) {
                        filename = header.substr(filename_pos, filename_end - filename_pos);
                        std::cout << "Extracted filename: " << filename << std::endl;
                    }
                }
                break;
            }
            ++it;
        }

        if (filename.empty()) {
            filename = "default_upload.bin";
        }
        std::vector<char>::iterator content_start = it;
        while (content_start != end - 4) {
            if (*content_start == '\r' && *(content_start + 1) == '\n' &&
                *(content_start + 2) == '\r' && *(content_start + 3) == '\n') {
                content_start += 4;
                break;
            }
            ++content_start;
        }
        if (content_start == end) {
            std::cerr << "File content start not found" << std::endl;
            return false;
        }

        std::vector<char>::iterator content_end = content_start;
        while (content_end != end - boundary.size()) {
            if (std::equal(boundary.begin(), boundary.end(), content_end)) {
                break;
            }
            ++content_end;
        }

        if (content_end == end) {
            std::cerr << "File content end not found" << std::endl;
            return false;
        }
        std::vector<char> clean_body(content_start, content_end);
        requestBodyBin.swap(clean_body);
    }
    return true;
}


void Connection::processRequest() {
    size_t pos;
    std::string raw = requestBuffer;

    while ((pos = requestBuffer.find("\r\n\r\n")) != std::string::npos) {

        std::string request = requestBuffer.substr(0, pos + 4);
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

                    bool    found = false; 
                    for( std::vector<std::string>::const_iterator it = route->methods.begin(); it != route->methods.end(); ++it  )
                    {
                        if( httpRequest.getMethod() ==  *it)
                        { 
                            found = true;
                            break;
                        }
                    }
                    std::cout << "\n" <<  route->clientMaxBodySize << "==== check MAX NOT TRUE \n" << httpRequest.getContentLength();
                    if(!found)
                    {
                        std::cout << "\t\tNOT FOUND" << std::endl;
                        response.setStatus(405);
                        response.setBody(HttpResponse::getDefaultErrorPage(405));
                    } else if (httpRequest.getContentLength() > route->clientMaxBodySize) {
                        std::cerr << "Content length " << httpRequest.getContentLength() 
                                << " exceeds max body size " << route->clientMaxBodySize << std::endl;
                        response.setStatus(413);
                        response.setBody(HttpResponse::getDefaultErrorPage(413));
                    }  else if (httpRequest.getMethod() == "GET") {
                        if (httpRequest.getPath() == route->path)
                        {
                            std::cout << "++++++++";
                            FileHandler::handleGet(route->root + "/" + route->index, response, route->autoIndex, route->path);
                            
                        }
                        else if (httpRequest.getPath().find("cgi-bin") != std::string::npos)
                        {
                            FileHandler::handleCgi(*route, response, httpRequest, requestBodyBin);
                        }
                        else
                        {
                            std::string pathForGet = httpRequest.getPath();
                            while (route->path != "/" && pathForGet.find(route->path) != std::string::npos)
                            {
                                pathForGet = pathForGet.substr(route->path.length());
                            }
                            FileHandler::handleGet(route->root + pathForGet, response, route->autoIndex, route->path);
                        }
                    } else if (httpRequest.getMethod() == "POST") {
                        std::string uploadPath;
                        if (route->uploadStore.empty()) {
                            uploadPath = route->root;
                        } 
                        else 
                        {
                            uploadPath = route->uploadStore;
                            std::cout << "Using upload store path: " << uploadPath << std::endl;
                        }
                        std::cout << "Request body size: " << httpRequest.getBody().length() << std::endl;
   
                        FileHandler::handlePost(uploadPath, filename, response, requestBodyBin);
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
                            std::cout << deletePath << "delete\n\n";
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