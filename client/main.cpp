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
    // Test libraries
#endif
    
    return 0;
}