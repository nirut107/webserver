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
#include <cctype>
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
		const ServerConfig* 	config;
		std::string 			requestBuffer;
		std::vector<char> 		requestBodyBin;
		std::vector<char> 		requestOnlyBodyBin;
		std::string 			responseBuffer;
		std::string				filename;
		time_t 					lastActivity;
		bool 					keepAlive;
		
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
		void RequestCutOffBody(const std::string& headers, std::vector<char>& requestBodyBin);
		bool appendRequestData(const std::string& data, int socket);
		void setBodyBin(std::vector<char> body, std::string& header);
		void processRequest();
		void updateLastActivity();
};

#endif