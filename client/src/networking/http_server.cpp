#include "../../include/networking/http_server.hpp"

#include <exception>

namespace networking::http {
    Server::Server(int port, int maxCon)
    : serverCon_(port, maxCon) {}

    void Server::run() {
        pfds_.push_back({
            serverCon_.getSocket(),
            POLLIN,
            0
        });

        while (true) {
            poll(pfds_.data(), pfds_.size(), -1);

            for (size_t i = 0; i < pfds_.size(); i++) {
                auto tempFd = pfds_[i];

                if (!(tempFd.revents & POLLIN)) continue;

                if (tempFd.fd == serverCon_.getSocket()) {
                    incomingConnection();
                } else {
                    handleRequest(tempFd.fd, i);
                }
            }
        }
    }

    void Server::incomingConnection() {
        auto con = serverCon_.acceptConnection();

        if (con) {
            pfds_.push_back({con->fd, POLLIN, 0});
            clients_[con->fd] = std::move(*con);
        }
    }

    void Server::handleRequest(int fd, size_t& index) {
        auto& client = clients_[fd];
        
        auto res = tcp::readData(fd, client.buff, INC_DATA_MAX_SIZE_BYTES);

        switch (res) {
            case tcp::L_TCP_SOCKET_RES::SUCCESSFUL_READ:
                // PARSE HTTP
                break;

            case tcp::L_TCP_SOCKET_RES::CLIENT_CON_CLOSED:
                closeConnection(fd, index);
                break;

            case tcp::L_TCP_SOCKET_RES::BLOCKING:
                return;

            default:
                throw std::runtime_error("Socket entered unexpected state");
        }
    }

    void Server::closeConnection(int fd, size_t& index) {
        close(fd);
        clients_.erase(fd);
        pfds_.erase(pfds_.begin() + index);

        if (index > 0) {
            index--;
        }
    }
}