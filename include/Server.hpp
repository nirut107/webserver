/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:33:49 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:33:49 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <signal.h>
#include "ServerConfig.hpp"
#include "ConnectionManager.hpp"
#include "FileHandler.hpp"

#define INFO_COLOR "\033[32m"
#define RESET_COLOR "\033[0m" 
#define ERROR_COLOR "\033[31m"
#define REQUEST_COLOR "\033[34m"

class Server {
private:
    std::vector<int> serverSockets;
    std::vector<ServerConfig> configs;
    ConnectionManager connectionManager;
    bool running;

public:
    Server();
    ~Server();
    
    void start(const std::vector<ServerConfig>& configs);
    void run();
    void stop();
    void handleRequest(int clientSocket, const RouteConfig* route);
};

#endif