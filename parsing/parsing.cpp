#include <cctype>
#include "Server.hpp"

// Function to trim whitespace from both ends
std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

// Function to remove a trailing semicolon if it exists
std::string removeTrailingSemicolon(const std::string &str) {
    std::string trimmed = trim(str);
    if (!trimmed.empty() && trimmed[trimmed.size() - 1] == ';') {
        return trimmed.substr(0, trimmed.size() - 1);
    }
    return trimmed;
}

std::vector<std::string> readConfigFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(file, line)) {
        line = line.substr(0, line.find("#")); // Remove comments
        if (!line.empty()) lines.push_back(line);
    }

    return lines;
}

std::vector<Server> parseConfig(const std::string &filename) {
    std::ifstream file(filename.c_str());  // Ensure compatibility with C++98
    std::string line;
    std::vector<Server> servers;
    Server current_server;
    t_location current_location;
    bool in_location = false;
    std::string location_path;
    bool in_server = false;

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return servers;
    }

    while (std::getline(file, line)) {
        line = trim(line.substr(0, line.find("#"))); // Remove comments and trim
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "server") {
            if (current_server.isValidServer()) servers.push_back(current_server);
            current_server = Server(); // Start a new server block
        } else if (key == "listen") {
            int port;
            iss >> port;
            current_server.setPort(port);
        } else if (key == "server_name") {
            std::string name;
            std::getline(iss, name); // Get the rest of the line
            current_server.setServerName(removeTrailingSemicolon(trim(name)));
        } else if (key == "host") {
            std::string host;
            std::getline(iss, host);
            current_server.setHost(removeTrailingSemicolon(trim(host)));
        } else if (key == "root") {
            std::string root;
            std::getline(iss, root);
            root = removeTrailingSemicolon(trim(root));
            if (in_location) current_location.root = root;
            else current_server.setRoot(root);
        } else if (key == "index") {
            std::string index;
            std::getline(iss, index);
            index = removeTrailingSemicolon(trim(index));
            if (in_location) current_location.index = index;
            else current_server.setIndex(index);
        } else if (key == "error_page") {
            int code;
            std::string path;
            iss >> code;
            std::getline(iss, path);
            current_server.setErrorPage(code, removeTrailingSemicolon(trim(path)));
        } else if (key == "location") {
            if (in_location) current_server.setLocation(location_path, current_location);
            iss >> location_path;
            location_path = trim(location_path);
            current_location = t_location();
            current_location.path = location_path;
            in_location = true;
        } else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) current_location.allow_methods.push_back(removeTrailingSemicolon(method));
        } else if (key == "autoindex") {
            std::string value;
            iss >> value;
            current_location.autoindex = (removeTrailingSemicolon(value) == "on");
        } else if (key == "cgi_path") {
            std::string path;
            while (iss >> path) current_location.cgi_path.push_back(removeTrailingSemicolon(path));
        } else if (key == "cgi_ext") {
            std::string ext;
            while (iss >> ext) current_location.cgi_ext.push_back(removeTrailingSemicolon(ext));
        } else if (key == "return") {
            std::string path;
            std::getline(iss, path);
            current_location.return_path = removeTrailingSemicolon(trim(path));
        } else if (line == "}") {
            if (in_location) {
                current_server.setLocation(location_path, current_location);
                in_location = false;
            } else if (in_server) {
                servers.push_back(current_server); // Push only when closing a server block
                in_server = false;
            }
        }
    }


    return servers;
}
