/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   plugin.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:23:25 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:23:25 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "plugin.hpp"

Plugin::Plugin() {}

Plugin::~Plugin() {}

void Plugin::initialize() {}

void Plugin::shutdown() {}

void Plugin::handleRequest(HttpRequest& request, HttpResponse& response) {
    (void)request;
    (void)response;
}

