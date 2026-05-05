#include <iostream>

#include "include/http_server/tcp_con.hpp"

int main(void) {
    
    networking::tcp::ServerCon server(8080, 20);

    auto client = server.acceptConnection();

    if (client.has_value()) {
        networking::tcp::byte_array data();
        auto res = client->readData(data, data.size());

        if (res == networking::tcp::L_TCP_SOCKET_RES::SUCCESSFUL_READ ||
            res == networking::tcp::L_TCP_SOCKET_RES::BLOCKING) {
                std::cout << "Incoming data: ";
                for (const char c : data) {
                    std::cout << c;
                }

                std::cout << std::endl;
            }
        
        
        res = client->sendData(data);
    }

    return 0;
}