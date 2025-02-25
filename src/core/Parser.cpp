/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 04:39:36 by schongte          #+#    #+#             */
/*   Updated: 2025/02/09 21:05:12 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

static std::string trim(const std::string &s) {
	std::string r = s;
	while (!r.empty() && (r[0] == ' ' || r[0] == '\t'))
		r.erase(0, 1);
	while (!r.empty() && (r[r.size()-1] == ' ' || r[r.size()-1] == '\t' || r[r.size()-1] == '\r' || r[r.size()-1] == '\n'))
		r.erase(r.size()-1, 1);
	return r;
}

static std::string trimSemicolon(const std::string &s) {
	std::string r = trim(s);
	while (!r.empty() && (r[r.size()-1] == ';' || r[r.size()-1] == ' ' || r[r.size()-1] == '\t' || r[r.size()-1] == '\r' || r[r.size()-1] == '\n'))
		r.erase(r.size()-1, 1);
	return trim(r);
}

static std::pair<std::string, std::string> parseDirectiveLine(const std::string &line) {
	std::string directive, value;
	size_t pos = line.find_first_of(" \t");
	
	if (pos != std::string::npos) {
		directive = line.substr(0, pos);
		value = line.substr(pos + 1);
		
		// Trim directive and value
		directive = trim(directive);
		value = trimSemicolon(value);
	} else {
		directive = trim(line);
		value = "";
	}
	
	return std::make_pair(directive, value);
}

Parser::Parser() {}
Parser::~Parser() {}

std::vector<ServerConfig> Parser::parseConfig(const std::string &filePath) {
	std::vector<ServerConfig> servers;
	std::ifstream ifs(filePath.c_str());
	std::string line;
	ServerConfig current;
	bool inServerBlock = false;
	bool inLocationBlock = false;
	RouteConfig currentRoute;

	while (std::getline(ifs, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		
		if (line.find("server") == 0 && line.find("{") != std::string::npos) {
			if (inServerBlock) {
				if (inLocationBlock) {
					current.routes.push_back(currentRoute);
					inLocationBlock = false;
				}
				servers.push_back(current);
				std::cout << "Added server with " << current.routes.size() << " routes" << std::endl;
			}
			inServerBlock = true;
			current = ServerConfig();
			current.clientMaxBodySize = 0;
			std::cout << "Found server block" << std::endl;
			continue;
		}
		
		if (inServerBlock) {
			if (line.find("location") == 0 && line.find("{") != std::string::npos) {
				if (inLocationBlock) {
					current.routes.push_back(currentRoute);
					std::cout << "Added route with path: " << currentRoute.path << std::endl;
				}
				inLocationBlock = true;
				currentRoute = RouteConfig();
				currentRoute.autoIndex = false;
				
				std::string pathPart = line.substr(8);
				pathPart = trim(pathPart);
				size_t bracePos = pathPart.find("{");
				if (bracePos != std::string::npos) {
					pathPart = pathPart.substr(0, bracePos);
				}
				currentRoute.path = trim(pathPart);
				std::cout << "Found location block with path: " << currentRoute.path << std::endl;
				continue;
			}
			if (line == "}") {
				if (inLocationBlock) {
					std::cout << "End of location block" << std::endl;
					current.routes.push_back(currentRoute);
					std::cout << "Added route with path: " << currentRoute.path
							 << ", root: " << currentRoute.root
							 << ", index: " << currentRoute.index << std::endl;
					inLocationBlock = false;
				} else {
					std::cout << "End of server block" << std::endl;
					if (!current.routes.empty() || !current.serverNames.empty() || current.port != 0) {
						servers.push_back(current);
						std::cout << "Added server with " << current.routes.size() << " routes" << std::endl;
					}
					inServerBlock = false;
				}
				continue;
			}
			
			std::pair<std::string, std::string> directive_value = parseDirectiveLine(line);
			std::string directive = directive_value.first;
			std::string value = directive_value.second;
			
			if (!inLocationBlock) {
				if (directive == "listen") {
					size_t colon = value.find(":");
					if (colon != std::string::npos) {
						current.host = value.substr(0, colon);
						current.port = std::atoi(value.substr(colon + 1).c_str());
					} else {
						current.port = std::atoi(value.c_str());
						current.host = "0.0.0.0"; // Default host if not specified
					}
					std::cout << "Found listen directive: " << current.host << ":" << current.port << std::endl;
				} else if (directive == "host") {
					current.host = value;
					std::cout << "Found host directive: " << value << std::endl;
				} else if (directive == "server_name") {
					std::istringstream nameStream(value);
					std::string name;
					while (nameStream >> name) {
						current.serverNames.push_back(name);
					}
				} else if (directive == "error_page") {
					std::istringstream errorStream(value);
					std::string code, page;
					errorStream >> code;
					std::getline(errorStream, page);
					current.errorPages[std::atoi(code.c_str())] = trim(page);
				} else if (directive == "client_max_body_size") {
					
					current.clientMaxBodySize = static_cast<size_t>(std::atoi(value.c_str()));
					current.clientMaxBodySize *= 1024 * 1000;
				}
			} else {
				if (directive == "methods" || directive == "allowed_methods") {
					std::istringstream methodStream(value);
					std::string method;
					while (methodStream >> method) {
						currentRoute.methods.push_back(method);
					}
					std::cout << "Added methods to route" << std::endl;
				} else if (directive == "autoindex") {
					if (value == "on") {
						currentRoute.autoIndex = true;
						std::cout << "Set autoindex to true" << std::endl;
					}
				} else if (directive == "root") {
					currentRoute.root = value;
					std::cout << "Set root to: '" << currentRoute.root << "' (length: " << currentRoute.root.length() << ")" << std::endl;
					for (size_t i = 0; i < currentRoute.root.length(); ++i) {
						std::cout << "char[" << i << "]: '" << currentRoute.root[i] << "' (ascii: " << (int)currentRoute.root[i] << ")" << std::endl;
					}
				} else if (directive == "index") {
					currentRoute.index = value;
					std::cout << "Set index to: '" << currentRoute.index << "' (length: " << currentRoute.index.length() << ")" << std::endl;
				} else if (directive == "cgi_extension") {
					currentRoute.cgiExtension = value;
				} else if (directive == "upload_store") {
					currentRoute.uploadStore = value;
				} else if (directive == "client_max_body_size") {
					
					currentRoute.clientMaxBodySize = static_cast<size_t>(std::atoi(value.c_str()));
					currentRoute.clientMaxBodySize *= 1024 * 1000;
				}
			}
		}
	}

	if (inLocationBlock) {
		current.routes.push_back(currentRoute);
	}
	if (inServerBlock && (!current.routes.empty() || !current.serverNames.empty() || current.port != 0)) {
		servers.push_back(current);
	}

	ifs.close();
	std::cout << "Parsed " << servers.size() << " server configurations" << std::endl;
	return servers;
}
