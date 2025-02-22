/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:24:41 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:24:41 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>
#include <unistd.h>
#include <algorithm>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>

void FileHandler::handleGet(const std::string& path, HttpResponse& response, bool autoIndex) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        response.setStatus(404);
        response.setBody(HttpResponse::getDefaultErrorPage(404));
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!autoIndex) {
            response.setStatus(403);
            response.setBody(HttpResponse::getDefaultErrorPage(403));
            return;
        }
        response.setBody(generateDirectoryListing(path));
        response.setHeader("Content-Type", "text/html");
        response.setStatus(200);
        return;
    }

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        response.setStatus(404);
        response.setBody(HttpResponse::getDefaultErrorPage(404));
        return;
    }

    char buffer[4096];
    std::string content;
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytesRead);
    }

    close(fd);

    if (bytesRead < 0) {
        response.setStatus(500);
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    }

    response.setBody(content);
    response.setHeader("Content-Type", getMimeType(path));
    response.setStatus(200);
}

void FileHandler::handlePost(const std::string& path, const std::string& content, HttpResponse& response) {
    std::string dirPath = path.substr(0, path.find_last_of('/'));
    
    std::cout << "POST: Creating directory path: " << dirPath << std::endl;

    // if (!dirPath.empty() && access(dirPath.c_str(), F_OK) != 0) {
    //     if (!createDirectories(dirPath)) {
    //         std::cerr << "Failed to create directory: " << dirPath << " - " << strerror(errno) << std::endl;
    //         response.setStatus(500);
    //         response.setBody("500 Internal Server Error - Failed to create directory");
    //         return;
    //     }
    //     std::cout << "Created directory: " << dirPath << std::endl;
    // }

    int pipefds[2];
    if (pipe(pipefds) == -1) {
        std::cerr << "Pipe creation failed\n";
            return;
    }
    pid_t pid = fork();
    if (path.find("/cgi-bin/") != std::string::npos)
    {
        if (pid == -1) 
        {
            std::cerr << "Fork failed\n";
        } 
        else if (pid == 0) 
        {
            close(pipefds[0]);
            if (dup2(pipefds[1], STDOUT_FILENO) == -1) 
            {
                std::cerr << "dup2 failed\n";
                exit(1);
            }
            if (dup2(pipefds[1], STDOUT_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
            }
            close(pipefds[1]);

            const char* args[] = 
            {
                (char*)"python3", 
                path.c_str(),
                NULL
            };

            if (execve("/usr/bin/python3", (char**)args, NULL) == -1) 
            {
                std::cerr << "Execve failed: " << strerror(errno) << "\n";
                exit(1);
            }
        } 
        else 
        {
            close(pipefds[1]);

            std::string output;
            char buffer[1024];
            ssize_t bytesRead;
        
            while ((bytesRead = read(pipefds[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                output += buffer;
            }

            close(pipefds[0]);
            wait(NULL);
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(output);
        }
        return ;
    }

    if (pid == -1) 
    {
        std::cerr << "Fork failed\n";
    } else if (pid == 0) 
    {
        close(pipefds[1]);
        if (dup2(pipefds[0], STDIN_FILENO) == -1) 
        {
            std::cerr << "dup2 failed\n";
            exit(1);
        }
        close(pipefds[0]);
        const char* args[] = 
        {
            (char*)"python3", 
            (char*)"upload.py",
            path.c_str(),
            NULL
        };

        if (execve("/usr/bin/python3", (char**)args, NULL) == -1) 
        {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
    } 
    else 
    {
        close(pipefds[0]);
        write(pipefds[1], content.c_str(), content.size());
        close(pipefds[1]);
    }
    // if (fd < 0) {
    //     std::cerr << "Failed to open file for writing: " << path << " - " << strerror(errno) << std::endl;
    //     response.setStatus(500);
    //     response.setBody("500 Internal Server Error - Failed to open file");
        // return;
    

    // ssize_t written = write(fd, content.c_str(), content.length());
    // close(fd);

    // if (written < 0 || static_cast<size_t>(written) != content.length()) {
    //     std::cerr << "Failed to write file content: " << strerror(errno) << std::endl;
    //     response.setStatus(500);
    //     response.setBody("500 Internal Server Error - Failed to write file");
    //     return;
    // }

    // std::cout << "Successfully wrote " << written << " bytes to " << path << std::endl;
    // response.setStatus(201);
    // response.setBody("File created successfully");
    // response.setHeader("Content-Type", "text/plain");
}

void FileHandler::handleCgi(const std::string& path, const std::string& content, HttpResponse& response)
{
    int pipefds[2];
    if (pipe(pipefds) == -1) {
        std::cerr << "Pipe creation failed\n";
            return;
    }
    pid_t pid = fork();
        if (pid == -1) 
        {
            std::cerr << "Fork failed\n";
        } 
        else if (pid == 0) 
        {
            close(pipefds[0]);
            if (dup2(pipefds[1], STDOUT_FILENO) == -1) 
            {
                std::cerr << "dup2 failed\n";
                exit(1);
            }
            if (dup2(pipefds[1], STDOUT_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
            }
            close(pipefds[1]);

            const char* args[] = 
            {
                (char*)"python3", 
                path.c_str(),
                content.c_str(),
                NULL
            };

            if (execve("/usr/bin/python3", (char**)args, NULL) == -1) 
            {
                std::cerr << "Execve failed: " << strerror(errno) << "\n";
                exit(1);
            }
        } 
        else 
        {
            close(pipefds[1]);

            std::string output;
            char buffer[1024];
            ssize_t bytesRead;
        
            while ((bytesRead = read(pipefds[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                output += buffer;
            }

            close(pipefds[0]);
            wait(NULL);
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(output);
        }
        return ;
}

void FileHandler::handleDelete(const std::string& path, HttpResponse& response) {
    if (access(path.c_str(), F_OK) != 0) {
        response.setStatus(404);
        response.setBody(HttpResponse::getDefaultErrorPage(404));
        return;
    }

    if (unlink(path.c_str()) != 0) {
        response.setStatus(500);
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    }

    response.setStatus(200);
    response.setBody("File deleted successfully");
}

bool FileHandler::isDirectory(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string FileHandler::generateDirectoryListing(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return "";
    }

    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html><head><title>Directory listing for " << path << "</title>"
         << "<style>body{font-family:Arial,sans-serif;margin:40px;line-height:1.6}"
         << "table{width:100%;border-collapse:collapse;margin-top:20px}"
         << "th,td{padding:8px;text-align:left;border-bottom:1px solid #ddd}"
         << "tr:hover{background-color:#f5f5f5}</style></head>"
         << "<body><h1>Directory listing for " << path << "</h1>"
         << "<table><tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        struct stat st;
        std::string fullPath = path + "/" + name;
        if (stat(fullPath.c_str(), &st) == 0) {
            html << "<tr><td><a href=\"" << name << "\">" << name << "</a></td>"
                << "<td>" << st.st_size << " bytes</td>"
                << "<td>" << ctime(&st.st_mtime) << "</td></tr>";
        }
    }

    html << "</table></body></html>";
    closedir(dir);
    return html.str();
}

std::string FileHandler::getMimeType(const std::string& path) {
    std::string ext = path.substr(path.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "txt") return "text/plain";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";
    if (ext == "pdf") return "application/pdf";
    if (ext == "zip") return "application/zip";
    if (ext == "xml") return "application/xml";

    return "application/octet-stream";
}

bool FileHandler::createDirectories(const std::string& path) {
    std::string current;
    size_t pos = 0;
    
    while ((pos = path.find('/', pos)) != std::string::npos) {
        current = path.substr(0, pos);
        if (!current.empty() && access(current.c_str(), F_OK) != 0) {
            if (mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
                std::cerr << "Failed to create directory " << current << ": " << strerror(errno) << std::endl;
                return false;
            }
        }
        pos++;
    }
    
    if (!path.empty() && access(path.c_str(), F_OK) != 0) {
        if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
            std::cerr << "Failed to create directory " << path << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    
    return true;
}
