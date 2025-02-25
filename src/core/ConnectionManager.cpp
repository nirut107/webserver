/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:24:27 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:24:27 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionManager.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <ctime>
#include <cerrno>

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager() {
    for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); ++it) {
        close(it->first);
    }
}

void ConnectionManager::setConfigs(const std::vector<ServerConfig>& configs) {
    this->configs = configs;
}

void ConnectionManager::addConnection(int socket, const ServerConfig& config) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0 || fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(socket);
        return;
    }
    
    connections[socket] = Connection(socket, config);
}

void ConnectionManager::closeConnection(int socket) {
    std::map<int, Connection>::iterator it = connections.find(socket);
    if (it != connections.end()) {
        close(socket);
        connections.erase(it);
    }
}

void ConnectionManager::addToPoll(std::vector<pollfd>& fds) {
    for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); ++it) {
        short events = POLLIN;
        if (it->second.hasDataToWrite()) {
            events |= POLLOUT;
        }
        pollfd pfd = {it->first, events, 0};
        fds.push_back(pfd);
    }
}

Connection* ConnectionManager::getConnection(int socket) {
    std::map<int, Connection>::iterator it = connections.find(socket);
    return it != connections.end() ? &it->second : NULL;
}

void ConnectionManager::handleRead(Connection& conn) {
    char buffer[40000];
    ssize_t bytesRead;
    while (true) {
        bytesRead = recv(conn.getSocket(), buffer, sizeof(buffer), 0);

        if (bytesRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            std::cerr << "recv() error: " << strerror(errno) << std::endl;
            closeConnection(conn.getSocket());
            return;
        }
        if (bytesRead == 0) {
            std::cout << "Client disconnected\n";
            closeConnection(conn.getSocket());
            return;
        }
        
        std::string raw(buffer, bytesRead);
        size_t headerEnd = raw.find("\r\n\r\n");
        std::vector<char> body;
        std::string headers = raw.substr(0, headerEnd + 4);

        
        if (headers.size() < raw.size()) {
            size_t bodyStart = headerEnd + 4;
            body.assign(buffer + bodyStart, buffer + raw.size());
            conn.setBodyBin(body, headers);
            conn.processRequest();
            return ;
        }
        else
        {
            if (conn.appendRequestData(std::string(buffer), conn.getSocket())) {
                conn.processRequest();
                return ;
            }
        }
    }
    if (bytesRead == 0) {
        closeConnection(conn.getSocket());
        return;
    }
    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        closeConnection(conn.getSocket());
        return;
    }
}

void ConnectionManager::handleWrite(Connection& conn) {
    if (!conn.hasDataToWrite()) return;
    
    const std::string& response = conn.getResponse();
    ssize_t bytesSent = send(conn.getSocket(), response.c_str(), response.length(), MSG_DONTWAIT);
    
    if (bytesSent > 0) {
        conn.clearResponse(bytesSent);
    } else if (bytesSent == 0 || (bytesSent < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        closeConnection(conn.getSocket());
    }
}

void ConnectionManager::checkTimeouts() {
    time_t now = time(NULL);
    std::vector<int> timeoutSockets;
    
    for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (now - it->second.getLastActivityTime() > CONNECTION_TIMEOUT) {
            timeoutSockets.push_back(it->first);
        }
    }
    
    for (size_t i = 0; i < timeoutSockets.size(); ++i) {
        closeConnection(timeoutSockets[i]);
    }
}

const RouteConfig* ConnectionManager::findRoute(const ServerConfig& config, const HttpRequest& request) const {
    std::string path = request.getPath();
    
    for (std::vector<RouteConfig>::const_iterator it = config.routes.begin(); it != config.routes.end(); ++it) {
        if (path.find(it->path) == 0) {
            return &(*it);
        }
    }
    
    return NULL;
} 
