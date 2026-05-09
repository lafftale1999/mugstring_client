#include "../../include/networking/http_server.hpp"

#include <exception>

#include "../../include/networking/http_parser.hpp"
#include "../../include/networking/http_router.hpp"

namespace networking::http {
    Server::Server(int port, int maxCon)
    : serverCon_(port, maxCon) {}

    Server::~Server() {
        for (auto& [fd, client] : clients_) {
            close(client.fd);
        }

        clients_.clear();
        pfds_.clear();
    }

    void Server::run(std::atomic<bool>& running) {
        pfds_.push_back({
            serverCon_.getSocket(),
            POLLIN,
            0
        });

        while (running) {
            int ready = poll(pfds_.data(), pfds_.size(), -1);

            if (ready < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("poll() failed");
            }

            if (ready == 0) continue;

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

        auto res = tcp::readData(fd, client.buf, INC_DATA_MAX_SIZE_BYTES - client.buf.size());

        if (res == tcp::L_TCP_SOCKET_RES::CLIENT_CON_CLOSED) {
            closeConnection(fd, index);
            return;
        }

        if (res == tcp::L_TCP_SOCKET_RES::BUFFER_FULL) {
            client.buf.clear();
            Response response(RESPONSE_CODE::CONTENT_TOO_LARGE, {}, {});
            return;
        }
        
        // Check if we've received all headers
        std::size_t hPos = 0;
        if (Request::headersReceived(client.buf, hPos) != 0) return;

        uint16_t err = Request::bodyReceived(client.buf, hPos);
        if (err == 1) return; // body incomplete
        else if (err >= 100) {
            client.buf.clear();
            Response response(static_cast<RESPONSE_CODE>(err), {}, {});
            tcp::sendData(fd, response.buildResponse());
            return; // Request denied
        }

        // Complete request received
        Request request(client.buf);
        client.buf.clear();
        Response response = router_.handleRequest(request);
        tcp::sendData(fd, response.buildResponse());
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