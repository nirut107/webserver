/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:25:35 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:25:35 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ConnectionManager.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <iostream>

Server::Server() : serverSockets(), configs(), connectionManager(), running(true) {
    signal(SIGPIPE, SIG_IGN);
}

Server::~Server() {
    for (size_t i = 0; i < serverSockets.size(); ++i) {
        close(serverSockets[i]);
    }
}

void Server::start(const std::vector<ServerConfig>& configs) {
    this->configs = configs;
    connectionManager.setConfigs(configs);
    
    for (size_t i = 0; i < configs.size(); ++i) {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
        }

        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(serverSocket);
            throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
        }

        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(configs[i].port);
        std::cout << "Binding to " << configs[i].host << ":" << configs[i].port << std::endl;
        if (inet_pton(AF_INET, configs[i].host.c_str(), &serverAddr.sin_addr) <= 0) {
            close(serverSocket);
            throw std::runtime_error("Invalid address: " + configs[i].host);
        }

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::string error = "Failed to bind socket: ";
            error += strerror(errno);
            std::stringstream ss;
            ss << " (host=" << configs[i].host << ", port=" << configs[i].port << ")";
            error += ss.str();
            close(serverSocket);
            throw std::runtime_error(error);
        }

        // Get the actual bound address
        struct sockaddr_in boundAddr;
        socklen_t boundLen = sizeof(boundAddr);
        if (getsockname(serverSocket, (struct sockaddr*)&boundAddr, &boundLen) == 0) {
            char boundIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &boundAddr.sin_addr, boundIP, INET_ADDRSTRLEN);
            std::cout << "Actually bound to " << boundIP << ":" << ntohs(boundAddr.sin_port) << std::endl;
        }

        if (listen(serverSocket, SOMAXCONN) < 0) {
            close(serverSocket);
            throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
        }

        int flags = fcntl(serverSocket, F_GETFL, 0);
        if (flags < 0 || fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
            close(serverSocket);
            throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
        }

        std::cout << "Server listening on " << configs[i].host << ":" << configs[i].port << std::endl;
        serverSockets.push_back(serverSocket);
    }
}

void Server::handleRequest(int clientSocket, const RouteConfig* route) {
    HttpRequest request;
    HttpResponse response;

    try {
        std::cout << "Handling request on socket " << clientSocket << std::endl;
        if (!request.parse(clientSocket)) {
            std::cerr << "Failed to parse request" << std::endl;
            response.setStatus(400);
            response.setBody("400 Bad Request");
            response.send(clientSocket);
            return;
        }
        std::cout << "Request parsed successfully: " << request.getMethod() << " " << request.getPath() << std::endl;

        if (!route) {
            std::cerr << "No matching route found" << std::endl;
            response.setStatus(404);
            response.setBody("404 Not Found");
            response.send(clientSocket);
            return;
        }
        std::cout << "Route found: " << route->path << std::endl;

        if (std::find(route->methods.begin(), route->methods.end(), request.getMethod()) == route->methods.end()) {
            std::cerr << "Method not allowed: " << request.getMethod() << std::endl;
            response.setStatus(405);
            response.setBody("405 Method Not Allowed");
            response.send(clientSocket);
            return;
        }

        if (request.getContentLength() > route->clientMaxBodySize) {
            std::cerr << "Request entity too large" << std::endl;
            response.setStatus(413);
            response.setBody("413 Request Entity Too Large");
            response.send(clientSocket);
            return;
        }

        std::cout << "Processing request..." << std::endl;
        if (request.getMethod() == "GET") {
            FileHandler::handleGet(route->root + request.getPath(), response, route->autoIndex, route->path);
        } else if (request.getMethod() == "POST") {
            std::string uploadPath;
            if (route->uploadStore.empty()) {
                uploadPath = route->root + request.getPath();
                std::cout << "Using root path for upload: " << uploadPath << std::endl;
            } else {
                std::string filename = request.getPath();
                if (filename.find(route->path) == 0) {
                    filename = filename.substr(route->path.length());
                }
                if (!filename.empty() && filename[0] == '/') {
                    filename = filename.substr(1);
                }
                std::cout << "Creating upload directory: " << route->uploadStore << std::endl;
                if (!FileHandler::createDirectories(route->uploadStore)) {
                    std::cerr << "Failed to create upload directory" << std::endl;
                    response.setStatus(500);
                    response.setBody("500 Internal Server Error");
                    return;
                }
                uploadPath = route->uploadStore + "/" + filename;
                std::cout << "Using upload store path: " << uploadPath << std::endl;
            }
            std::cout << "Request body size: " << request.getBody().length() << std::endl;
            FileHandler::handlePost(uploadPath, request.getBody(), response);
        } else if (request.getMethod() == "DELETE") {
            FileHandler::handleDelete(route->root + request.getPath(), response);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        response.setStatus(500);
        response.setBody("500 Internal Server Error");
    }

    std::cout << "Sending response..." << std::endl;
    if (!response.send(clientSocket)) {
        std::cerr << "Failed to send response" << std::endl;
    }
}

void Server::run() {
    std::cout << "Starting server with " << serverSockets.size() << " listening sockets" << std::endl;
    
    std::vector<pollfd> fds;
    fds.reserve(1024); // Pre-allocate space for efficiency
    
    while (running) {
        fds.clear();

        for (size_t i = 0; i < serverSockets.size(); ++i) {
            pollfd pfd = {serverSockets[i], POLLIN, 0};
            fds.push_back(pfd);
        }
        
  
        connectionManager.addToPoll(fds);
        
        //std::cout << "Polling " << fds.size() << " file descriptors..." << std::endl;
        
        int pollResult = poll(fds.data(), fds.size(), 1000); //(1 second)
        if (pollResult < 0) {
            if (errno == EINTR) {
                std::cout << "Poll interrupted by signal, continuing..." << std::endl;
                continue;
            }
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            break;
        }
        
        if (pollResult == 0) {
            continue;
        }
        
        std::cout << "Poll returned " << pollResult << " events" << std::endl;
        
        // Handle events
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents == 0) continue;
            
            std::cout << "Event on fd " << fds[i].fd << ": " << fds[i].revents << std::endl;
            
            // Check if this is a server socket
            bool isServerSocket = false;
            for (size_t j = 0; j < serverSockets.size(); ++j) {
                if (fds[i].fd == serverSockets[j]) {
                    isServerSocket = true;
                    if (fds[i].revents & POLLIN) {
                        struct sockaddr_in clientAddr;
                        socklen_t clientLen = sizeof(clientAddr);
                        int clientSocket = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &clientLen);
                        
                        if (clientSocket >= 0) {
                            std::cout << "Accepted new connection on socket " << clientSocket << std::endl;
                            // Set non-blocking mode for client socket
                            int flags = fcntl(clientSocket, F_GETFL, 0);
                            if (flags >= 0) {
                                fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
                                connectionManager.addConnection(clientSocket, configs[j]);
                            } else {
                                std::cerr << "Failed to set non-blocking mode: " << strerror(errno) << std::endl;
                                close(clientSocket);
                            }
                        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
                        }
                    }
                    if (fds[i].revents & (POLLERR | POLLHUP)) {
                        std::cerr << "Error on server socket " << fds[i].fd << std::endl;
                    }
                    break;
                }
            }
            
            // If not a server socket, it's a client connection
            if (!isServerSocket) {
                Connection* conn = connectionManager.getConnection(fds[i].fd);
                if (conn) {
                    if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                        std::cout << "Error on client socket " << fds[i].fd << std::endl;
                        connectionManager.closeConnection(fds[i].fd);
                    } else {
                        if (fds[i].revents & POLLIN) {
                            std::cout << "Data available on client socket " << fds[i].fd << std::endl;
                            connectionManager.handleRead(*conn);
                        }
                        if (fds[i].revents & POLLOUT) {
                            std::cout << "Can write to client socket " << fds[i].fd << std::endl;
                            connectionManager.handleWrite(*conn);
                        }
                    }
                }
            }
        }
        
        connectionManager.checkTimeouts();
    }
}

void Server::stop() {
    running = false;
} 
