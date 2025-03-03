/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: schongte <schongte@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 13:28:53 by schongte          #+#    #+#             */
/*   Updated: 2025/02/15 13:28:53 by schongte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP
#include <string>
#include <map>
#include <sstream>
#include <iostream>

#define INFO_COLOR "\033[32m"
#define RESET_COLOR "\033[0m" 
#define ERROR_COLOR "\033[31m"
#define REQUEST_COLOR "\033[34m"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void setStatus(int code);
    void setHeader(const std::string& name, const std::string& value);   
    void setBody(const std::string& content);
    void setContentType(const std::string& type) { setHeader("Content-Type", type); }
    bool send(int clientSocket);
    int  getStatus(){return statusCode;};
    std::string serialize() const;
    void clear();

    static std::string getDefaultErrorPage(int code);
    static std::string getStatusText(int code);
    std::string getHeader(const std::string& name) const;

private:
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
};
#endif
