#include <iostream>

#include "include/http_server/tcp_con.hpp"

int main(void) {
    
    networking::ServerCon server(8080, 20);

    auto client = server.acceptConnection();

    if (client.has_value()) {
        networking::tcp::byte_array data;
        auto res = networking::tcp::readData(client->fd, data, 2000);

        if (res == networking::tcp::L_TCP_SOCKET_RES::SUCCESSFUL_READ ||
            res == networking::tcp::L_TCP_SOCKET_RES::BLOCKING) {
            std::cout << "Incoming data: ";
            for (const char c : data) {
                std::cout << c;
            }
            std::cout << '\n';
        } else {
            std::cerr << "Failed to read incoming request: " << (int)res << std::endl;
            std::cerr << "Reason: " << strerror(errno) << std::endl;
        }

        res = client->sendData(data);
        if (res != networking::tcp::L_TCP_SOCKET_RES::SUCCESSFUL_SEND) {
            std::cerr << "Failed to send data\n";
        }
    }

    return 0;
}