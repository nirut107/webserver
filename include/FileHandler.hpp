/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:28:29 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:28:29 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include "HttpResponse.hpp"
#include <string>
#include <sys/wait.h>
#include <vector>
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

class FileHandler {
public:
    static void handleGet(const std::string& path, HttpResponse& response, bool autoIndex, std::string rootPath);
    static void handlePost(const std::string& path, const std::string& filename, HttpResponse& response, std::vector<char> requestBodyBin);
    static void handleCgi(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::vector<char> 	requestBodyBin);
    static void handleDelete(const std::string& path, HttpResponse& response);
    static bool isDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);

private:
    static std::string generateDirectoryListing(const std::string& path, std::string rootPath);
    static std::string getMimeType(const std::string& path);
};

#endif
