/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:25:28 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:25:28 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include <algorithm>
#include <iostream>

Router::Router() {}
Router::~Router() {}

const RouteConfig* Router::findRoute(const ServerConfig& config, const std::string& path) {
    // std::cout << "\nFinding route for path: '" << path << "'" << std::endl;
    // std::cout << "Available routes:" << std::endl;
    for (std::vector<RouteConfig>::const_iterator it = config.routes.begin(); it != config.routes.end(); ++it) {
        // std::cout << "- Path: '" << it->path << "'" << std::endl;
        // std::cout << "  Methods: ";
        for (std::vector<std::string>::const_iterator mit = it->methods.begin(); mit != it->methods.end(); ++mit) {
            // std::cout << *mit << " ";
        }
        // std::cout << std::endl;
        // std::cout << "  Root: " << it->root << std::endl;
        // std::cout << "  Upload store: " << it->uploadStore << std::endl;
        // std::cout << it->index << std::endl;
    }
    // std::cout << std::endl;
    
    for (std::vector<RouteConfig>::const_iterator it = config.routes.begin(); it != config.routes.end(); ++it) {
        if (path == it->path) {
            // std::cout << "Found exact matching route: '" << it->path << "'" << std::endl;
            return &(*it);
        }
    }
    
    const RouteConfig* bestMatch = NULL;
    size_t bestMatchLength = 0;
    
    for (std::vector<RouteConfig>::const_iterator it = config.routes.begin(); it != config.routes.end(); ++it) {
        // std::cout << "Checking if '" << path << "' starts with '" << it->path << "'" << std::endl;
        
        if (it->path == "/upload" && path.find("/upload/") == 0) {
            // std::cout << "Found /upload match" << std::endl;
            return &(*it);
        }

        if (it->path.length() > path.length()) {
            // std::cout << "Skipping (route path too long)" << std::endl;
            continue;
        }

        if (path.find(it->path) == 0) {
            // std::cout << "Found prefix match" << std::endl;
            if (it->path.length() > bestMatchLength) {
                bestMatchLength = it->path.length();
                bestMatch = &(*it);
                // std::cout << "New best match: '" << it->path << "' (length: " << bestMatchLength << ")" << std::endl;
            }
        }
    }
    
    if (bestMatch) {
        // std::cout << "Using best matching route: '" << bestMatch->path << "'" << std::endl;
        // std::cout << "Route methods:";
        for (std::vector<std::string>::const_iterator mit = bestMatch->methods.begin(); mit != bestMatch->methods.end(); ++mit) {
            // std::cout << " " << *mit;
        }
        // std::cout << std::endl;
        // std::cout << "Upload store: " << bestMatch->uploadStore << std::endl;
        return bestMatch;
    }
    
    // std::cout << "No matching route found" << std::endl;
    return NULL;
} 



