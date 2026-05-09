#include "../../include/networking/http_server.hpp"

#include <exception>

#include "../../include/networking/http_parser.hpp"

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
        tcp::byte_array buf;

        auto res = tcp::readData(fd, buf, INC_DATA_MAX_SIZE_BYTES - client.buf.size());

        if (res == tcp::L_TCP_SOCKET_RES::CLIENT_CON_CLOSED) {
            closeConnection(fd, index);
            return;
        }

        if (res == tcp::L_TCP_SOCKET_RES::BUFFER_FULL) {
            client.buf.clear();
            // Send error message 413
        }
        
        client.buf.append(std::move(std::string(buf.begin(), buf.end())));
        
        // Check if we've received all headers
        static const std::string_view headerEndDelim("\r\n\r\n");
        auto headerEnd = client.buf.find(headerEndDelim);
        if (headerEnd == std::string::npos) return;

        // Check if we've received the body
        static const std::string_view clString = "Content-Length:";
        auto contentLengthPos = client.buf.find(clString);
        if (contentLengthPos != std::string::npos &&
            contentLengthPos < headerEnd)
            {
            auto valStart = contentLengthPos + clString.size();
            auto valEnd = client.buf.find("\r\n", valStart);
            size_t contentLength = std::stoi(client.buf.substr(valStart, valEnd - valStart));
            
            // Check if body is complete
            if (client.buf.size() < headerEnd + headerEndDelim.size() + contentLength) return;
        } else {
            client.buf.clear();
            // Send error message 413
        }

        // Complete request received
        Request request(client.buf);
        
        client.buf.clear();

        Response response;
        response.setHeader("Content-Type", "application/text");
        response.setResponseCode(RESPONSE_CODE::OK);
        response.setBody("Hello world");

        tcp::sendData(fd, response.buildByteResponse());
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