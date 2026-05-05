#ifndef LTALE_HTTP_SERVER_HPP_
#define LTALE_HTTP_SERVER_HPP_

#include "tcp_con.hpp"

#include <iostream>
#include <map>

#include <poll.h>

namespace networking::http {

static constexpr size_t INC_DATA_MAX_SIZE_BYTES = (2048 * 10);

class Server {
public:
    Server(int port, int maxCon);

    void run();

private:
    
    tcp::ServerCon              serverCon_;
    std::vector<pollfd>         pfds_;
    std::map<int, tcp::Client>  clients_;
    
    void incomingConnection();
    void handleRequest(int fd, size_t& index);
    void closeConnection(int fd, size_t& index);
};
}

#endif