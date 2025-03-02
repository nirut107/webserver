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


static void handlePhpHeader(std::string &input)
{
    std::cout << "\033[31mDEBUG ME - handlePhpHeader called _\033[0m";
    std::istringstream inputStream(input);
    std::ostringstream outputStream;
    std::string line;

    while (std::getline(inputStream, line)) {
        if (line.find("Content-type:") != 0) 
            outputStream << line << "\n"; 
    }
    input = outputStream.str();
}


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

static bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

static std::string getUniqueFilename(const std::string& path, const std::string& filename) {
    std::string baseName = filename;
    std::string extension;
    size_t dotPos = filename.find_last_of('.');

    if (dotPos != std::string::npos) {
        baseName = filename.substr(0, dotPos);
        extension = filename.substr(dotPos);
    }

    std::string uniqueName = filename;
    int count = 1;

    while (fileExists(path + "/" + uniqueName)) {
        std::ostringstream ss;
        ss << baseName << "(" << count << ")" << extension;
        uniqueName = ss.str();
        ++count;
    }
    return uniqueName;
}

// void FileHandler::handlePost(const std::string& path, const std::string& filename, HttpResponse& response, std::vector<char> requestBodyBin) {
    
//     std::string fullPath = path + "/" + getUniqueFilename(path, filename);;
    
//     response.setStatus(200);
//     response.setHeader("Content-Type", "application/json");
//     response.setHeader("Location", fullPath);
//     response.setBody();

// }

void FileHandler::handleUpload(const std::string& path, const std::string& filename, HttpResponse& response, std::vector<char> requestBodyBin) {
    
    std::string fullPath = path + "/" + getUniqueFilename(path, filename);;

    if (requestBodyBin.empty()) {
        response.setStatus(400);
        response.setHeader("Content-Type", "text/plain");
        response.setBody("No file uploaded");
        return;
    }

    std::ofstream file(fullPath.c_str(), std::ios::binary);
    if (!file) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Failed to open file for writing");
        return;
    }

    file.write(&requestBodyBin[0], requestBodyBin.size());
    if (!file) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Failed to write file");
        return;
    }
    
    file.close();
    
    response.setStatus(201);
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Location", fullPath);
    response.setBody("Upload " + filename + " successful");

}

void FileHandler::handleCgis(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& 	requestBodyBin, std::string ext , std::string cmd)
{
    if ( ext == ".php")
        return FileHandler::handlePhpCgi(route, response, httpRequest, requestBodyBin, ext, cmd);
    if  (ext == ".py")
        return FileHandler::handlePythonCgi(route, response, httpRequest, requestBodyBin , ext , cmd );
    response.setStatus(500);
    response.setBody(HttpResponse::getDefaultErrorPage(500));

}



void FileHandler::handlePythonCgi(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBodyBin , std::string ext , std::string cmd)
{
    std::string pathWithCgi;
    if (httpRequest.getPath() == route.path)
    {
        pathWithCgi = route.root + "/" + route.index;
    }
    else
    {
       pathWithCgi = httpRequest.getPath();
    }

    if (httpRequest.getPath().find("/cookie") != std::string::npos) {
        std::string strBody(requestBodyBin.size(), '\0');
        std::copy(requestBodyBin.begin(), requestBodyBin.end(), strBody.begin());
        FileHandler::handleCookie(route, response, httpRequest, strBody);
        return ;
    }

    while (pathWithCgi.find("/cgi-bin") != std::string::npos)
    {
        pathWithCgi = pathWithCgi.substr(route.path.length());
    }
    std::string cgiPath = route.root + pathWithCgi;
    // std::cout << " Run cgi with path " << cgiPath << std::endl;
    if (route.uploadStore.empty())
    {
        size_t lastSlash = cgiPath.find_last_of('/');
        route.uploadStore = cgiPath.substr(0, lastSlash + 1);
    }

    std::string fullCmdPath = cmd;
    std::string cmdPath = fullCmdPath.substr( fullCmdPath.find_last_of("/") + 1, fullCmdPath.length());


    int pipefds_out[2];
    int pipefds_in[2];

    if (pipe(pipefds_out) == -1 || pipe(pipefds_in) == -1) {
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
        close(pipefds_out[0]);
        close(pipefds_in[1]);

        if (dup2(pipefds_out[1], STDOUT_FILENO) == -1 ||
            dup2(pipefds_out[1], STDERR_FILENO) == -1 ||
            dup2(pipefds_in[0], STDIN_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }

        close(pipefds_out[1]);
        close(pipefds_in[0]);

        const char* args[] = {
            cmdPath.c_str(),
            cgiPath.c_str(),
            NULL
        };

        std::stringstream length;
        length << httpRequest.getContentLength();

        std::string envMethod = "REQUEST_METHOD=" + httpRequest.getMethod();
        std::string envQuery = "QUERY_STRING=" + httpRequest.getQuery();
        std::string envContentType = "CONTENT_TYPE=" + httpRequest.getContentType();
        std::string envContentLength = "CONTENT_LENGTH=" + length.str();
        std::string envUploadDir = "UPLOAD_DIR=" + route.uploadStore;
        std::string envFileSize = "HTTP_FILESIZE=" + length.str();
        std::string envStatus = "REDIRECT_STATUS=200";


        char* envp[] = {
            (char*)(envMethod.c_str()),
            (char*)(envQuery.c_str()),
            (char*)(envContentLength.c_str()),
            (char*)envContentType.c_str(),
            (char*)(envUploadDir.c_str()),
            (char*)(envFileSize.c_str()),
            NULL
        };

        if (execve(fullCmdPath.c_str(), (char**)args, envp) == -1) {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
        exit(0);
    } 
    else {
        close(pipefds_out[1]);
        close(pipefds_in[0]);

        if (!requestBodyBin.empty()) {
            ssize_t written = write(pipefds_in[1], requestBodyBin.c_str(), requestBodyBin.size());
            if (written == -1) {
                std::cerr << "Failed to write request body to CGI process\n";
            }
        }
        close(pipefds_in[1]);

        int status;
        time_t startTime = time(NULL);
        bool finished = false;

        while (true) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                finished = true;
                break;
            } else if (result == 0) { 
                if (time(NULL) - startTime > FHANDLER_CGI_TIMEOUT_SEC) {
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                    break;
                }
                usleep(100000);
            } else {
                break;
            }
        }

        std::string output;
        char buffer[FHANDLER_OUTPUT_BUFFER_SIZE];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefds_out[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        close(pipefds_out[0]);

        if (finished && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(output);
        } else {
            response.setStatus(500);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(500));
        }
    }
}

void FileHandler::handleCookie(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBody)
{
    
    std::string     session;
    std::string     sessionID;
    char            times[100];
    bool            generate = false;

    std::string     key = "session-id=";
    size_t          pos = httpRequest.getCookie().find(key);

    if (httpRequest.getContentLength() > 0)
    {
        generate = true;
    }
    else if (pos != std::string::npos)
    {
        pos += key.length();
        size_t end = httpRequest.getCookie().find(";", pos);
        sessionID = httpRequest.getCookie().substr(pos, end - pos);
        sessionID = route.uploadStore + "/" + sessionID;
        struct stat buffer;
        if (stat(session.c_str(), &buffer) != 0)
        {
            session = sessionID;
        }
    }

    if (generate) {
        const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        
        std::srand(std::time(0));

        for (size_t i = 0; i < 32; ++i) {
            sessionID += chars[std::rand() % chars.size()];
        }

        session = route.uploadStore + "/" + sessionID;
        std::time_t now = std::time(NULL);
        now +=  60;
        std::tm *gmt_time = std::gmtime(&now);
        std::strftime(times, sizeof(times), "%a, %d %b %Y %H:%M:%S GMT", gmt_time);

        std::ofstream outfile(session.c_str());
        if (outfile) {
            outfile << "name=" + requestBody;
            outfile << "expire=" + std::string(times);
            outfile.close();
        }
        
    }
    
    int pipefds_out[2];
    int pipefds_in[2];

    if (pipe(pipefds_out) == -1 || pipe(pipefds_in) == -1) {
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
        close(pipefds_out[0]);
        close(pipefds_in[1]);

        if (dup2(pipefds_out[1], STDOUT_FILENO) == -1 ||
            dup2(pipefds_out[1], STDERR_FILENO) == -1 ||
            dup2(pipefds_in[0], STDIN_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }

        close(pipefds_out[1]);
        close(pipefds_in[0]);

        const char* args[] = {
            (char*)"python3",
            (char*)"cookie/session.py",
            (char*)session.c_str(),
            NULL
        };

        std::stringstream length;
        length << httpRequest.getContentLength();

        std::string envMethod = "REQUEST_METHOD=" + httpRequest.getMethod();
        std::string envQuery = "QUERY_STRING=" + httpRequest.getQuery();
        std::string envContentType = "CONTENT_TYPE=" + httpRequest.getContentType();
        std::string envContentLength = "CONTENT_LENGTH=" + length.str();
        std::string envUploadDir = "UPLOAD_DIR=" + route.uploadStore;
        std::string envFileSize = "HTTP_FILESIZE=" + length.str();
        std::string envStatus = "REDIRECT_STATUS=200";


        char* envp[] = {
            (char*)(envMethod.c_str()),
            (char*)(envQuery.c_str()),
            (char*)(envContentLength.c_str()),
            (char*)(envUploadDir.c_str()),
            (char*)(envFileSize.c_str()),
            NULL
        };

    
        if (execve((char*)"/usr/bin/python3", (char**)args, envp) == -1) {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
        exit(0);
    } 
    else {
        close(pipefds_out[1]);
        close(pipefds_in[0]);

        
        ssize_t written = write(pipefds_in[1], session.c_str(), session.size());
            if (written == -1) {
                std::cerr << "Failed to write request body to CGI process\n";
            }
        }
        close(pipefds_in[1]);

        int status;
        time_t startTime = time(NULL);
        bool finished = false;

        while (true) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                finished = true;
                break;
            } else if (result == 0) { 
                if (time(NULL) - startTime > FHANDLER_CGI_TIMEOUT_SEC) {
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                    break;
                }
                usleep(FHANDLER_CGI_USLEEP_MSEC);
            } else {
                break;
            }
        }

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefds_out[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        close(pipefds_out[0]);

        if (finished && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            if (!sessionID.empty())
            {
                response.setHeader("Set-Cookie", "session-id=" + sessionID + ";" + " Expires=" + std::string(times) + ";" + " Secure; HttpOnly;" );
            }
            response.setBody(output);
        } else {
            response.setStatus(500);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(500));
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


void FileHandler::handlePhpCgi(RouteConfig route, HttpResponse& response, const HttpRequest httpRequest, std::string& requestBodyBin, std::string ext, std::string cmd)
{

    std::string pathWithCgi;
    if (httpRequest.getPath() == route.path)
        pathWithCgi = route.root + "/" + route.index;        
    else
        pathWithCgi = route.root + "/" + httpRequest.getPath();
       
    // std::string cgiPath = route.root + pathWithCgi;
    std::string cgiPath = pathWithCgi;

    // std::cout << "\033[31mDEBUG ME  === route.root: _" << route.root << "_" << std::endl;
    // std::cout << "\033[31mDEBUG ME  === pathWithCgi: _" << pathWithCgi << "_" << std::endl;


    if (route.uploadStore.empty())
    {
        size_t lastSlash = cgiPath.find_last_of('/');
        route.uploadStore = cgiPath.substr(0, lastSlash + 1);
    }
    std::string cmdPath;
    std::string fullCmdPath;
    fullCmdPath = cmd;
    cmdPath = fullCmdPath.substr( fullCmdPath.find_last_of("/") + 1, fullCmdPath.length());
    // std::cout << "\033[31mDEBUG ME  === cgiPath: _" << cgiPath << "_" << std::endl;
   
    int pipefds_out[2];
    int pipefds_in[2];

    if (pipe(pipefds_out) == -1 || pipe(pipefds_in) == -1) {
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
        close(pipefds_out[0]);
        close(pipefds_in[1]);

        if (dup2(pipefds_out[1], STDOUT_FILENO) == -1 ||
            dup2(pipefds_out[1], STDERR_FILENO) == -1 ||
            dup2(pipefds_in[0], STDIN_FILENO) == -1) {
            std::cerr << "dup2 failed\n";
            exit(1);
        }

        close(pipefds_out[1]);
        close(pipefds_in[0]);
        
        const char* args[] = {
            cmdPath.c_str(),
            cgiPath.c_str(),            
            NULL
        };

        std::stringstream length;
        length << httpRequest.getContentLength();

        

        std::string envMethod = "REQUEST_METHOD=" + httpRequest.getMethod();
        std::string envQuery = "QUERY_STRING=" + httpRequest.getQuery();
        std::string envContentType = "CONTENT_TYPE=" + httpRequest.getContentType();        
        std::string envContentLength = "CONTENT_LENGTH=" + length.str();
        std::string envRedirectStatus = "REDIRECT_STATUS=200";
        std::string envScriptFilname = "SCRIPT_FILENAME=" + cgiPath;
        
        char* envp[] = {
            (char *)(envScriptFilname.c_str()), 
            (char *)(envMethod.c_str()),
            (char *)(envQuery.c_str()),
            (char *)(envContentLength.c_str()),
            (char *)(envContentType.c_str()), 
            (char *)(envContentLength.c_str()), 
            (char *)(envRedirectStatus.c_str()),
            NULL
        };
        if (execve(fullCmdPath.c_str(), (char**)args, envp) == -1) {
            std::cerr << "Execve failed: " << strerror(errno) << "\n";
            exit(1);
        }
        exit(0);
    } 
    else {

        close(pipefds_out[1]);
        close(pipefds_in[0]);
        if (!requestBodyBin.empty()) {
            std::vector<char> rawBody = httpRequest.getRawBody();
            ssize_t written = write(pipefds_in[1], rawBody.data(), rawBody.size());
            if (written == -1) {
                std::cerr << "Failed to write request body to CGI process\n";
            }
        }
        close(pipefds_in[1]);

        int status;
        time_t startTime = time(NULL);
        bool finished = false;

        while (true) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                finished = true;
                break;
            } else if (result == 0) { 
                if (time(NULL) - startTime > FHANDLER_CGI_TIMEOUT_SEC) {
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                    break;
                }
                usleep(FHANDLER_CGI_USLEEP_MSEC);
            } else {
                break;
            }
        }

        std::string output;
        char buffer[FHANDLER_OUTPUT_BUFFER_SIZE];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefds_out[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        close(pipefds_out[0]);
        if (finished && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // extra function to handle PHP default 200 status
            handlePhpHeader(output);    

            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(output);
        } else {
            response.setStatus(500);
            response.setHeader("Content-Type", "text/html");
            response.setBody(HttpResponse::getDefaultErrorPage(500));
        }
    }
}