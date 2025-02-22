/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:27:55 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:27:55 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <fstream> 
#include <sys/socket.h>
#include <sstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>


class Connection {
	private:
		int socket;
		const ServerConfig* config;
		std::string requestBuffer;
		std::string responseBuffer;
		time_t lastActivity;
		bool keepAlive;
		
	public:
		Connection();
		Connection(int socket, const ServerConfig& config);
		Connection(const Connection& other);
		Connection& operator=(const Connection& other);
		
		int getSocket() const;
		time_t getLastActivityTime() const;
		bool hasDataToWrite() const;
		const std::string& getResponse() const;
		void clearResponse(size_t bytes);
		bool appendRequestData(const std::string& data, int socket);
		void processRequest();
		void updateLastActivity();
};

#endif