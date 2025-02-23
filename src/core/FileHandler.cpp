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

void FileHandler::handleGet(const std::string& path, HttpResponse& response, bool autoIndex, std::string rootPath) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        response.setStatus(404);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(404));
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!autoIndex) {
            response.setStatus(403);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(403));
            return;
        }
        response.setBody(generateDirectoryListing(path, rootPath));
        response.setHeader("Content-Type", "text/html");
        response.setStatus(200);
        return;
    }

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        response.setStatus(404);
        response.setHeader("Content-Type", "text/html");
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
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    }

    response.setBody(content);
    response.setHeader("Content-Type", getMimeType(path));
    response.setStatus(200);
}

void FileHandler::handlePost(const std::string& path, const std::string& content, HttpResponse& response) {
    

    int pipefds[2];
    
    if (pipe(pipefds) == -1) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(500));
    } else if (pid == 0) 
    {
        close(pipefds[1]);
        if (dup2(pipefds[0], STDIN_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }
        close(pipefds[0]);
        const char* args[] = {
            (char*)"python3", 
            (char*)"upload.py",
            path.c_str(),
            NULL
        };

        if (execve("/usr/bin/python3", (char**)args, NULL) == -1) {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
    } 
    else {
        int status;

        close(pipefds[0]);
        write(pipefds[1], content.c_str(), content.size());
        close(pipefds[1]);
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                response.setStatus(200);
                response.setHeader("Content-Type", "text/plain");
                response.setBody("File uploaded successfully\n");
                return ;
        }
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            switch (exit_code) {
                case 0:
                    response.setStatus(200);
                    response.setHeader("Content-Type", "text/plain");
                    response.setBody("File uploaded successfully\n");
                    break;
                case 1:
                    response.setStatus(500);
                    response.setHeader("Content-Type", "text/html");
                    response.setBody(HttpResponse::getDefaultErrorPage(500));
                    break;
                case 2:
                    response.setStatus(413);
                    response.setHeader("Content-Type", "text/html");
                    response.setBody(HttpResponse::getDefaultErrorPage(413));
                    break;
                case 3:
                    response.setStatus(415);
                    response.setHeader("Content-Type", "text/html");
                    response.setBody(HttpResponse::getDefaultErrorPage(415));
                    break;
                case 4:
                    response.setStatus(403);
                    response.setHeader("Content-Type", "text/html");
                    response.setBody(HttpResponse::getDefaultErrorPage(403));
                    break;
                case 5:
                    response.setStatus(200);
                    response.setHeader("Content-Type", "text/plain");
                    response.setBody("No file uploaded. Please try again\n");
                    break;
                default:
                    response.setStatus(500);
                    response.setHeader("Content-Type", "text/html");
                    response.setBody(HttpResponse::getDefaultErrorPage(500));
                    break;
            }
        }
    }
}

void FileHandler::handleCgi(const std::string& path, const std::string& content, HttpResponse& response)
{
    int pipefds[2];
    if (pipe(pipefds) == -1) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(500));
        return;
    } 
    else if (pid == 0) {
        close(pipefds[0]);
        if (dup2(pipefds[1], STDOUT_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }
        if (dup2(pipefds[1], STDERR_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }
        close(pipefds[1]);

        const char* args[] = {
            (char*)"python3",
            path.c_str(),
            content.c_str(),
            NULL
        };

        if (execve("/usr/bin/python3", (char**)args, NULL) == -1) {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
        exit(0);
    } 
    else {
        close(pipefds[1]);
        int status;
        waitpid(pid, &status, 0);

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefds[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        close(pipefds[0]);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(output);
        } else {
            std::cout << WIFEXITED(status) << " " << WEXITSTATUS(status) << output;
            response.setStatus(500);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(500));
        }
    }
}

void FileHandler::handleDelete(const std::string& path, HttpResponse& response) {
    if (access(path.c_str(), F_OK) != 0) {
        response.setStatus(404);
        response.setHeader("Content-Type", "text/html");
        response.setBody(HttpResponse::getDefaultErrorPage(404));
        return;
    }

    if (unlink(path.c_str()) != 0) {
        if (errno == EACCES || errno == EPERM) {
            response.setStatus(403);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(403));
        } else {
            response.setStatus(500);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(500));
        }
        return;
    }

    response.setStatus(200);
    response.setHeader("Content-Type", "text/plain");
    response.setBody("File deleted successfully");
}

bool FileHandler::isDirectory(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string FileHandler::generateDirectoryListing(const std::string& path, std::string rootPath) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return "";
    }

    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n";
    html << "<head>\n";
    html << "    <meta charset=\"UTF-8\">\n";
    html << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "    <title >Directory listing for " << path << "</title>\n";
    html << "    <style>\n";
    html << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n";
    html << "        body {\n";
    html << "            font-family: 'Arial', sans-serif;\n";
    html << "            background: linear-gradient(to bottom, #0B1026 0%, #1B2735 50%, #090A0F 100%);\n";
    html << "            color: #fff;\n";
    html << "            min-height: 100vh;\n";
    html << "            padding: 20px;\n";
    html << "            position: relative;\n";
    html << "            overflow-x: hidden;\n";
    html << "        }\n";
    html << "        table {\n";
    html << "            width: 100%;\n";
    html << "            border-collapse: collapse;\n";
    html << "            margin-top: 20px;\n";
    html << "            box-shadow: 0 0 15px rgba(126, 182, 255, 0.3);\n";
    html << "            background: rgba(255, 255, 255, 0.05);\n";
    html << "            border-radius: 10px;\n";
    html << "            overflow: hidden;\n";
    html << "        }\n";
    html << "        th, td {\n";
    html << "            width: 30%;\n";
    html << "            padding: 12px;\n";
    html << "            text-align: left;\n";
    html << "            border-bottom: 1px solid rgba(255, 255, 255, 0.2);\n";
    html << "        }\n";
    html << "        th {\n";
    html << "            background: rgba(255, 255, 255, 0.1);\n";
    html << "            color: #7EB6FF;\n";
    html << "            text-transform: uppercase;\n";
    html << "        }\n";
    html << "        .container { max-width: 800px; margin: 0 auto; position: relative; magin-top: 30px;}\n";
    html << "        .ti { text-align: center;}\n";
    html << "        .back { text-align: right; margin-top: 20px;}\n";
    html << "        body::before {\n";
    html << "            content: '';\n";
    html << "            position: absolute;\n";
    html << "            width: 100%; height: 100%; top: 0; left: 0;\n";
    html << "            background-image:\n";
    html << "                radial-gradient(2px 2px at 20px 30px, #ffffff, rgba(0,0,0,0)),\n";
    html << "                radial-gradient(2px 2px at 40px 70px, #ffffff, rgba(0,0,0,0)),\n";
    html << "                radial-gradient(2px 2px at 50px 160px, #ffffff, rgba(0,0,0,0)),\n";
    html << "                radial-gradient(2px 2px at 90px 40px, #ffffff, rgba(0,0,0,0)),\n";
    html << "                radial-gradient(2px 2px at 130px 80px, #ffffff, rgba(0,0,0,0));\n";
    html << "            background-repeat: repeat;\n";
    html << "            animation: twinkle 5s infinite linear;\n";
    html << "            z-index: -1;\n";
    html << "        }\n";
    html << "        @keyframes twinkle {\n";
    html << "            from { transform: translateY(0); }\n";
    html << "            to { transform: translateY(-100px); }\n";
    html << "        }\n";
    html << "        a {\n";
    html << "            color: #7EB6FF;\n";
    html << "            text-decoration: none;\n";
    html << "            transition: all 0.3s ease;\n";
    html << "        }\n";
    html << "        a:hover {\n";
    html << "            color: #ffffff;\n";
    html << "            text-shadow: 0 0 10px rgba(126, 182, 255, 0.5);\n";
    html << "        }\n";
    html << "        @keyframes float {\n";
    html << "            0% { transform: translateY(0px) rotate(0deg); }\n";
    html << "            50% { transform: translateY(-20px) rotate(5deg); }\n";
    html << "            100% { transform: translateY(0px) rotate(0deg); }\n";
    html << "        }\n";
    html << "    </style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "    <div class=\"container\">\n";
    html << "       <h1 class=\"ti\">Directory listing for " <<  path << "</h1>\n";


    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        struct stat st;
        std::string herfPath;

        if (rootPath == "/"){
            herfPath = name;
        }
        else {
            herfPath = rootPath + "/" + name;
        }
        std::string fullPath = path + "/" + name;
        if (stat(fullPath.c_str(), &st) == 0) {
            html << "<table><tr><td><a href=\"" << herfPath<< "\">" << name << "</a></td>"
                << "<td>" << st.st_size << " bytes</td>"
                << "<td>" << ctime(&st.st_mtime) << "</td></tr>";
                
        }
        
    }
    size_t lastSlash = rootPath.find_last_of('/');
    if (lastSlash != std::string::npos && rootPath != "/") {
        rootPath = rootPath.substr(0, lastSlash + 1);
    }
    html << "</table>\n";
    html << "<div class=\"back\"><a  href=\"" << rootPath << "\"> Back</a><div></div>\n";
    html << "</body></html>";
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
