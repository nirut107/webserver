/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nusamank <nusamank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 16:19:22 by nusamank          #+#    #+#             */
/*   Updated: 2025/02/20 20:38:53 by nusamank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// Constructor
Server::Server() : _port(0) {}

Server::Server(Server const &src)
{
    _port = src._port;
    _server_name = src._server_name;
    _host = src._host;
    _root = src._root;
    _index = src._index;
    _error_page = src._error_page; // std::map has its own deep copy
    _locations = src._locations;   // std::map also does deep copy
}

Server &Server::operator=(Server const &src)
{
	if (this != &src) { // Check for self-assignment
        _port = src._port;
        _server_name = src._server_name;
        _host = src._host;
        _root = src._root;
        _index = src._index;
        _error_page = src._error_page;
        _locations = src._locations;
    }
    return *this;
}

// Destructor
Server::~Server() {}

// Setters
void Server::setPort(int port) { _port = port; }
void Server::setServerName(const std::string &name) { _server_name = name; }
void Server::setHost(const std::string &host) { _host = host; }
void Server::setRoot(const std::string &root) { _root = root; }
void Server::setIndex(const std::string &index) { _index = index; }
void Server::setErrorPage(int code, const std::string &path) { _error_page[code] = path; }
void Server::setLocation(const std::string &path, const t_location &loc) { _locations[path] = loc; }

// Getters
int Server::getPort() const { return _port; }
std::string Server::getServerName() const { return _server_name; }
std::string Server::getHost() const { return _host; }
std::string Server::getRoot() const { return _root; }
std::string Server::getIndex() const { return _index; }
std::map<int, std::string> Server::getErrorPages() const { return _error_page; }
std::map<std::string, t_location> Server::getLocations() const { return _locations; }


bool Server::isValidServer()
{
	return _port > 0 && !_server_name.empty() && !_root.empty();
}
