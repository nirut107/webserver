/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:26:11 by schongte          #+#    #+#             */
/*   Updated: 2025/02/08 17:08:19 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include "Server.hpp"
#include "Parser.hpp"

void printConfig(const std::vector<ServerConfig>& servers) {
	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "Server " << i + 1 << ":" << std::endl;
		std::cout << "    Host: " << servers[i].host << std::endl;
		std::cout << "    Port: " << servers[i].port << std::endl;
		std::cout << "    Max Body Size: " << servers[i].clientMaxBodySize << std::endl;
		
		std::cout << "    Server Names: ";
		for (size_t n = 0; n < servers[i].serverNames.size(); ++n) {
			std::cout << servers[i].serverNames[n] << " ";
		}
		std::cout << std::endl;
		
		std::cout << "    Error Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator it = servers[i].errorPages.begin();
			 it != servers[i].errorPages.end(); ++it) {
			std::cout << "        " << it->first << ": " << it->second << std::endl;
		}
		
		std::cout << "    Routes:" << std::endl;
		for (size_t r = 0; r < servers[i].routes.size(); ++r) {
			std::cout << "        Path: " << servers[i].routes[r].path << std::endl;
			std::cout << "        Root: " << servers[i].routes[r].root << std::endl;
			std::cout << "        Index: " << servers[i].routes[r].index << std::endl;
			std::cout << "        Methods: ";
			for (size_t m = 0; m < servers[i].routes[r].methods.size(); ++m) {
				std::cout << servers[i].routes[r].methods[m] << " ";
			}
			std::cout << std::endl;
			//std::cout << "        CGI Extension: " << servers[i].routes[r].cgiExtension << std::endl;
			std::cout << "        Upload Store: " << servers[i].routes[r].uploadStore << std::endl;
			std::cout << "        Auto Index: " << (servers[i].routes[r].autoIndex ? "on" : "off") << std::endl;
			std::cout << "        Max Body Size: " << servers[i].routes[r].clientMaxBodySize << std::endl;
		}
		std::cout << std::endl;
	}
}

int main(int argc, char* argv[]) {



	try {
		std::string configPath = (argc > 1) ? argv[1] : "config/default.conf";
		Parser parser;			
		std::vector<ServerConfig> servers = parser.parseConfig(configPath);

		if (argc > 2 && std::string(argv[2]) == "-t") {
			printConfig(servers);
			return 0;
		}
		Server server;
		server.start(servers);
		server.run();
				
		
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
