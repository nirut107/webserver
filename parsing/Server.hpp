/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nusamank <nusamank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 22:15:08 by nusamank          #+#    #+#             */
/*   Updated: 2025/02/20 21:48:07 by nusamank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <fstream>
# include <iostream>
# include <map>
# include <string>
# include <sstream>
# include <vector>

typedef struct s_location {
	std::string path;
	std::vector<std::string> allow_methods;
	bool autoindex;
	std::string index;
	std::string return_path;
	std::string root;
	std::vector<std::string> cgi_path;
	std::vector<std::string> cgi_ext;
} t_location;

class Server
{
	private:
		int _port;
		std::string _server_name;
		std::string _host;
		std::string _root;
		std::string _index;
		std::map<int, std::string> _error_page;
		std::map<std::string, t_location> _locations;


	public:
		Server();
		Server(Server const &src);
		Server &operator=(Server const &src);
		~Server();

		void setPort(int port);
		void setServerName(const std::string &name);
		void setHost(const std::string &host);
		void setRoot(const std::string &root);
		void setIndex(const std::string &index);
		void setErrorPage(int code, const std::string &path);
		void setLocation(const std::string &path, const t_location &loc);

		int getPort() const;
		std::string getServerName() const;
		std::string getHost() const;
		std::string getRoot() const;
		std::string getIndex() const;
		std::map<int, std::string> getErrorPages() const;
		std::map<std::string, t_location> getLocations() const;

		bool isValidServer();
};

std::vector<Server> parseConfig(const std::string &filename);

#endif