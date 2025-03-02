/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:33:44 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:33:44 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct RouteConfig {
    std::string                         path;
    std::string                         root;
    std::string                         index;
    std::vector<std::string>            methods;
    std::map<std::string,std::string>   cgiExtensions;
    std::string                         uploadStore;
    bool                                autoIndex;
    size_t                              clientMaxBodySize;
    int                                 redirectStatus;
    std::string                         redirectPath;

    RouteConfig() : autoIndex(false), clientMaxBodySize(1024 * 1024 * 8) {}
};

struct ServerConfig {
    std::string host;
    int port;
    std::vector<std::string> serverNames;
    std::vector<RouteConfig> routes;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;

    ServerConfig() : host("0.0.0.0"), port(0), clientMaxBodySize(1024 * 1024 * 8) {}
};

#endif 