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

class FileHandler {
public:
    static void handleGet(const std::string& path, HttpResponse& response, bool autoIndex);
    static void handlePost(const std::string& path, const std::string& content, HttpResponse& response);
    static void handleCgi(const std::string& path, const std::string& content, HttpResponse& response);
    static void handleDelete(const std::string& path, HttpResponse& response);
    static bool isDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);

private:
    static std::string generateDirectoryListing(const std::string& path);
    static std::string getMimeType(const std::string& path);
};

#endif 
