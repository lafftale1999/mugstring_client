#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <ctime>

#include <csignal>
#include <atomic>

#include "include/networking/tcp_con.hpp"
#include "include/networking/http_parser.hpp"
#include "include/networking/http_server.hpp"

#define TEST_SERVER

static std::atomic<bool> running = true;

static void signalHandler(int) {
    running = false;
}

int main(void) {
#ifdef TEST_SERVER
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    networking::http::Server server(8080, 20);
    server.run(running);

#elif
    std::ifstream file("request.txt");

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    std::string raw_request {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    std::cout << "----------------------------------------\n";
    std::cout << "Parsed" << std::endl;

    networking::http::Request request(raw_request);

    std::cout << "Method: " << request.getMethod().getMethodString() << std::endl;
    std::cout << "Path: " << request.getPath() << std::endl;
    std::cout << "Version: " << request.getVersion() << std::endl;

    for (const auto& h : request.getHeaders()) {
        std::cout << h.first << " : " << h.second << std::endl;
    }

    std::cout << "Body:\n" << request.getBody() << std::endl;

    std::cout << "----------------------------------------\n";

    std::cout << "TIME TEST" << std::endl;
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);

    std::cout << ctime(&timestamp);

    std::cout << "----------------------------------------\n";
    std::cout << "------------ RESPONSE ------------------\n";
    std::cout << "----------------------------------------\n";

    networking::http::Response response;
    response.setBody("Hello World");
    response.setHeader("Content-Type", "application/text");
    response.setResponseCode(networking::http::RESPONSE_CODE::OK);

    std::cout << response.buildStringResponse() << std::endl;std::ifstream file("request.txt");

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    std::string raw_request {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    std::cout << "----------------------------------------\n";
    std::cout << "Parsed" << std::endl;

    networking::http::Request request(raw_request);

    std::cout << "Method: " << request.getMethod().getMethodString() << std::endl;
    std::cout << "Path: " << request.getPath() << std::endl;
    std::cout << "Version: " << request.getVersion() << std::endl;

    for (const auto& h : request.getHeaders()) {
        std::cout << h.first << " : " << h.second << std::endl;
    }

    std::cout << "Body:\n" << request.getBody() << std::endl;

    std::cout << "----------------------------------------\n";

    std::cout << "TIME TEST" << std::endl;
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);

    std::cout << ctime(&timestamp);

    std::cout << "----------------------------------------\n";
    std::cout << "------------ RESPONSE ------------------\n";
    std::cout << "----------------------------------------\n";

    networking::http::Response response;
    response.setBody("Hello World");
    response.setHeader("Content-Type", "application/text");
    response.setResponseCode(networking::http::RESPONSE_CODE::OK);

    std::cout << response.buildStringResponse() << std::endl;
#endif
    
    return 0;
}