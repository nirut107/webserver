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
# define FILE_HANDLER_HPP

# include <string>
# include <sys/wait.h>
# include <vector>
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# define  FHANDLER_OUTPUT_BUFFER_SIZE   102400
# define  FHANDLER_CGI_USLEEP_MSEC        100000
# define  FHANDLER_CGI_TIMEOUT_SEC        5

class FileHandler {
public:
    static void handleGet(const std::string& path, HttpResponse& response, bool autoIndex, std::string rootPath);
    // static void handlePost(const std::string& path, const std::string& filename, HttpResponse& response, std::vector<char> requestBodyBin);
    static void handleCgis(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBodyBin, std::string ext , std::string cmd);
    static void handlePythonCgi(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& 	requestBodyBin, std::string ext , std::string cmd);
    static void handlePhpCgi(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBodyBin, std::string ext , std::string cmd);
    static void handleCookie(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBody);
    static void handleUpload(const std::string& path, const std::string& filename, HttpResponse& response, std::vector<char> requestBodyBin);
    static void handleDelete(const std::string& path, HttpResponse& response);
    static bool isDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);

private:
    static std::string generateDirectoryListing(const std::string& path, std::string rootPath);
    static std::string getMimeType(const std::string& path);
};

#endif
