/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PluginManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:34:00 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:34:00 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include <vector>
#include "../plugins/plugin.hpp"

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    void loadPlugin(Plugin* plugin);
    void initializePlugins();
    void shutdownPlugins();

private:
    std::vector<Plugin*> plugins;
};

#endif 
