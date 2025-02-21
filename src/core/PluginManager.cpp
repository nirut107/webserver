/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PluginManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:25:20 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:25:20 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PluginManager.hpp"

PluginManager::PluginManager() {}

PluginManager::~PluginManager() {
    for (std::vector<Plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
        delete *it;
    }
    plugins.clear();
}

void PluginManager::loadPlugin(Plugin* plugin) {
    plugins.push_back(plugin);
}

void PluginManager::initializePlugins() {
    for (std::vector<Plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
        (*it)->initialize();
    }
}

void PluginManager::shutdownPlugins() {
    for (std::vector<Plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
        (*it)->shutdown();
    }
} 
