/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:28:21 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:28:21 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <map>
#include <vector>
#include <poll.h>
#include "Connection.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"

#define CONNECTION_TIMEOUT 60

class ConnectionManager {
	private:
		std::map<int, Connection> connections;
		std::vector<ServerConfig> configs;
		
	public:
		ConnectionManager();
		~ConnectionManager();
		
		void setConfigs(const std::vector<ServerConfig>& configs);
		void addConnection(int socket, const ServerConfig& config);
		void closeConnection(int socket);
		void addToPoll(std::vector<pollfd>& fds);
		Connection* getConnection(int socket);
		void handleRead(Connection& conn);
		void handleWrite(Connection& conn);
		void checkTimeouts();
		const RouteConfig* findRoute(const ServerConfig& config, const HttpRequest& request) const;
};

#endif