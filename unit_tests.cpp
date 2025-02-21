#include "include/Parser.hpp"
#include "include/Server.hpp"
#include "include/HttpRequest.hpp"
#include "include/HttpResponse.hpp"
#include "include/Router.hpp"
#include "include/FileHandler.hpp"
#include "include/ConnectionManager.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

//documentation has been auto generated via extension

void testConfigParsing() {
    Parser parser;
    std::vector<ServerConfig> servers = parser.parseConfig("tests/test_config.conf");
    
    assert(servers.size() >= 2 && "Expected at least 2 server configurations");
    
    const ServerConfig& sc1 = servers[0];
    assert(sc1.host == "127.0.0.1" && "First server host mismatch");
    assert(sc1.port == 8080 && "First server port mismatch");
    assert(!sc1.serverNames.empty() && "First server names missing");
    assert(!sc1.routes.empty() && "First server routes missing");
    
    const RouteConfig& route1 = sc1.routes[0];
    assert(route1.methods.size() >= 2 && "First server methods missing");
    assert(route1.root == "/tmp/webserv_test" && "First server root mismatch");
    assert(route1.index == "index.html" && "First server index mismatch");
    assert(route1.cgiExtension == ".php" && "First server CGI extension mismatch");
    assert(route1.uploadStore == "/tmp/webserv_test/uploads" && "First server upload store mismatch");
    assert(route1.autoIndex && "First server autoindex should be on");
    
    const ServerConfig& sc2 = servers[1];
    assert(sc2.host == "0.0.0.0" && "Second server host mismatch");
    assert(sc2.port == 80 && "Second server port mismatch");
    assert(!sc2.routes.empty() && "Second server routes missing");
    assert(sc2.routes[0].methods.size() == 1 && "Second server methods mismatch");
    
    std::cout << "Config parsing tests passed" << std::endl;
}

void testServerConnection() {
    // Find an available port
    int testPort = 0;
    {
        int testSock = socket(AF_INET, SOCK_STREAM, 0);
        if (testSock < 0) {
            std::cerr << "Failed to create test socket: " << strerror(errno) << std::endl;
            assert(false && "Failed to create test socket");
        }

        struct sockaddr_in testAddr;
        testAddr.sin_family = AF_INET;
        testAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Try ports in range 9000-10000
        for (int port = 9000; port < 10000; ++port) {
            testAddr.sin_port = htons(port);
            if (bind(testSock, (struct sockaddr*)&testAddr, sizeof(testAddr)) == 0) {
                testPort = port;
                close(testSock);
                break;
            }
        }

        if (testPort == 0) {
            close(testSock);
            std::cerr << "No available ports found" << std::endl;
            assert(false && "No available ports found");
        }
    }

    std::cout << "Found available port: " << testPort << std::endl;

    // Create a simple server configuration
    std::vector<ServerConfig> servers;
    ServerConfig sc;
    sc.host = "127.0.0.1";
    sc.port = testPort;
    RouteConfig rc;
    rc.path = "/";
    rc.methods.push_back("GET");
    rc.root = "/tmp";
    rc.index = "index.html";
    rc.autoIndex = true;
    sc.routes.push_back(rc);
    servers.push_back(sc);
    
    std::cout << "Starting server connection test..." << std::endl;
    
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        assert(false && "Fork failed");
    }
    
    if (pid == 0) {
        // Child process
        std::cout << "Child process starting server..." << std::endl;
        Server server;
        try {
            server.start(servers);
            std::cout << "Server started successfully on port " << testPort << std::endl;
            server.run();
        } catch (const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
            exit(1);
        }
        exit(0);
    }
    
    // Parent process
    std::cout << "Parent process waiting for server to start..." << std::endl;
    sleep(1); // Give the server time to start
    
    // Create client socket
    std::cout << "Creating client socket..." << std::endl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        assert(false && "Socket creation failed");
    }
    
    // Connect to server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(testPort);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    std::cout << "Attempting to connect to port " << testPort << "..." << std::endl;
    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (result < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        close(sock);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        assert(false && "Connection failed");
    }
    
    std::cout << "Connected successfully, sending request..." << std::endl;
    std::string request;
    request.append("GET / HTTP/1.1");
    request.push_back('\x0d');
    request.push_back('\x0a');
    request.append("Host: localhost");
    request.push_back('\x0d');
    request.push_back('\x0a');
    request.push_back('\x0d');
    request.push_back('\x0a');
    
    std::cout << "Sending request: [" << request << "]" << std::endl;
    ssize_t sent = send(sock, request.c_str(), request.length(), 0);
    if (sent < 0) {
        std::cerr << "Send failed: " << strerror(errno) << std::endl;
        close(sock);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        assert(false && "Failed to send request");
    }
    std::cout << "Sent " << sent << " bytes" << std::endl;
    
    std::cout << "Request sent, waiting for response..." << std::endl;
    char buffer[4096];
    ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
        std::cerr << "Receive failed: " << strerror(errno) << std::endl;
        close(sock);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        assert(false && "No response received");
    }
    buffer[received] = '\0';
    
    std::cout << "Response received (" << received << " bytes): " << buffer << std::endl;
    assert(strstr(buffer, "HTTP/1.1") != NULL && "Invalid HTTP response");
    
    close(sock);
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    
    std::cout << "Server connection tests passed" << std::endl;
}

void testFileOperations() {
    system("pkill -f webserv");
    sleep(1);
    
    system("rm -rf /tmp/webserv_test");
    assert(system("mkdir -p /tmp/webserv_test") == 0 && "Failed to create test directory");
    assert(system("mkdir -p /tmp/webserv_test/uploads") == 0 && "Failed to create uploads directory");
    assert(system("chmod -R 777 /tmp/webserv_test") == 0 && "Failed to set directory permissions");
    assert(system("echo 'test content' > /tmp/webserv_test/test.txt") == 0 && "Failed to create test file");
    
    int port = 10000;
    bool portFound = false;
    for (; port < 11000; ++port) {
        int testSock = socket(AF_INET, SOCK_STREAM, 0);
        if (testSock < 0) continue;
        
        struct sockaddr_in testAddr;
        testAddr.sin_family = AF_INET;
        testAddr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &testAddr.sin_addr);
        
        if (bind(testSock, (struct sockaddr*)&testAddr, sizeof(testAddr)) == 0) {
            close(testSock);
            portFound = true;
            break;
        }
        close(testSock);
    }
    assert(portFound && "Could not find an available port");
    
    std::vector<ServerConfig> servers;
    ServerConfig sc;
    sc.host = "127.0.0.1";
    sc.port = port;
    sc.clientMaxBodySize = 8192;
    RouteConfig rc;
    rc.path = "/";
    rc.methods.push_back("GET");
    rc.methods.push_back("POST");
    rc.methods.push_back("DELETE");
    rc.root = "/tmp/webserv_test";
    rc.uploadStore = "/tmp/webserv_test/uploads";
    rc.autoIndex = true;
    rc.clientMaxBodySize = sc.clientMaxBodySize;
    sc.routes.push_back(rc);
    servers.push_back(sc);
    
    int pipefd[2];
    assert(pipe(pipefd) == 0 && "Failed to create pipe");
    
    pid_t pid = fork();
    assert(pid >= 0 && "Fork failed");
    
    if (pid == 0) {
        close(pipefd[0]);
        
        Server server;
        write(pipefd[1], "1", 1);
        close(pipefd[1]);
        
        alarm(10);
        
        server.start(servers);
        server.run();
        exit(0);
    }
    
    close(pipefd[1]);
    
    char buf[1];
    ssize_t n = read(pipefd[0], buf, 1);
    close(pipefd[0]);
    
    assert(n == 1 && "Server failed to start");
    
    sleep(1);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* request = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request, strlen(request), 0);
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';
        
        assert(strstr(buffer, "test content") != NULL && "GET test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* content = "uploaded content";
        std::ostringstream request;
        request << "POST /uploads/test.txt HTTP/1.1\r\n"
                << "Host: localhost\r\n"
                << "Content-Length: " << strlen(content) << "\r\n\r\n"
                << content;
        
        std::string requestStr = request.str();
        std::cout << "\nSending POST request:\n" << requestStr << std::endl;
        
        ssize_t sent = send(sock, requestStr.c_str(), requestStr.length(), 0);
        assert(sent == static_cast<ssize_t>(requestStr.length()) && "Failed to send complete request");
        std::cout << "Sent " << sent << " bytes" << std::endl;
        
        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        std::cout << "Waiting for response..." << std::endl;
        int ret = poll(&pfd, 1, 5000);
        std::cout << "Poll returned: " << ret << std::endl;
        assert(ret > 0 && "Timeout waiting for response");
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        std::cout << "Received " << received << " bytes" << std::endl;
        buffer[received] = '\0';
        std::cout << "Response:\n" << buffer << std::endl;
        
        assert(strstr(buffer, "201 Created") != NULL && "POST test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* request = "DELETE /uploads/test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request, strlen(request), 0);
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';
        
        std::cout << "DELETE Response: " << buffer << std::endl;
        assert(strstr(buffer, "200 OK") != NULL && "DELETE test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        std::string content(10000, 'X');
        std::ostringstream request;
        request << "POST /large_upload.txt HTTP/1.1\r\n"
                << "Host: localhost\r\n"
                << "Content-Length: " << content.length() << "\r\n\r\n"
                << content;
        
        std::string requestStr = request.str();
        ssize_t sent = send(sock, requestStr.c_str(), requestStr.length(), 0);
        assert(sent == static_cast<ssize_t>(requestStr.length()) && "Failed to send complete request");
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';
        
        assert(strstr(buffer, "413 Request Entity Too Large") != NULL && "Large POST test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* content = "test content";
        std::ostringstream request;
        request << "POST /nonexistent/test.txt HTTP/1.1\r\n"
                << "Host: localhost\r\n"
                << "Content-Length: " << strlen(content) << "\r\n\r\n"
                << content;
        
        std::string requestStr = request.str();
        ssize_t sent = send(sock, requestStr.c_str(), requestStr.length(), 0);
        assert(sent == static_cast<ssize_t>(requestStr.length()) && "Failed to send complete request");
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';
        
        assert(strstr(buffer, "201 Created") != NULL && "Directory creation POST test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* request = "DELETE /nonexistent.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request, strlen(request), 0);
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';

        assert(strstr(buffer, "404 Not Found") != NULL && "Non-existent file DELETE test failed");
        close(sock);
    }
    
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        assert(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "Failed to connect");
        
        const char* request = "PUT /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request, strlen(request), 0);
        
        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[received] = '\0';
        
        assert(strstr(buffer, "405 Method Not Allowed") != NULL && "Unsupported method test failed");
        close(sock);
    }
    
    kill(pid, SIGTERM);
    int status;
    waitpid(pid, &status, 0);
    
    system("rm -rf /tmp/webserv_test");
    
    std::cout << "File operation tests passed" << std::endl;
}

int main() {
    try {
        testConfigParsing();
        testServerConnection();
        testFileOperations();
        std::cout << "All tests passed successfully" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
