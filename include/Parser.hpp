/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 04:39:59 by schongte          #+#    #+#             */
/*   Updated: 2025/02/06 04:39:59 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "ServerConfig.hpp"
#include <vector>
#include <string>

class Parser {
public:
	Parser();
	~Parser();

	std::vector<ServerConfig> parseConfig(const std::string &filePath);

private:
	void parseServerBlock(std::vector<ServerConfig> &servers);
	void parseLocationBlock(RouteConfig &route);
	void parseServer(ServerConfig& config, std::string& line);
	void parseLocation(RouteConfig& config, std::string& line);
	void parseMethods(std::vector<std::string>& methods, std::string& line);
};

#endif
