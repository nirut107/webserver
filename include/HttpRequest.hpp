/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:28:37 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:28:37 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <sstream>

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();

    bool parse(const std::string& request);
    bool parse(int clientSocket);
    bool isComplete() const;
    
    const std::string& getMethod() const { return method; }
    const std::string& getPath() const { return path; }
    const std::string& getQuery() const { return query; }
    const std::string& getVersion() const { return version; }
    const std::string& getBody() const { return body; }
    const std::string& getContentType() const { return contenttype; }
    const std::string& getCookie() const { return cookie; }
    const std::map<std::string, std::string>& getHeaders() const { return headers; }
    std::string getHeader(const std::string& name) const;
    size_t getContentLength() const { return contentLength; }

private:
    bool parseRequestLine(const std::string& line);
    bool parseHeader(const std::string& line);
    bool parseHeaders(const std::string& headers);
    bool validateRequest();

    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::string body;
    std::string contenttype;
    std::string cookie;
    std::map<std::string, std::string> headers;
    bool complete;
    size_t contentLength;
};

#endif 

