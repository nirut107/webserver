/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plugin.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 17:10:15 by schongte          #+#    #+#             */
/*   Updated: 2025/01/29 17:10:15 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <string>
#include <vector>
#include <map>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Plugin {
public:
    Plugin();
    virtual ~Plugin();
    virtual void initialize();
    virtual void shutdown();
    virtual void handleRequest(HttpRequest& request, HttpResponse& response);
};

#endif

// this is a placeholder for any plugin class.
