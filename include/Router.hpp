/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:33:54 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:33:54 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include <string>

class Router {
public:
    Router();
    ~Router();

    static const RouteConfig* findRoute(const ServerConfig& config, const std::string& path);
};

#endif 