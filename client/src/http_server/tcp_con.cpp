#include "../../include/http_server/tcp_con.hpp"

#include <array>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <fcntl.h>

namespace networking::tcp {

L_TCP_SOCKET_RES readData(int fd, byte_array& dataOut, size_t dataSize) {
    dataOut.clear();
    std::array<char, TCP_BUFFER_SIZE> buff;
    size_t totalBytesRead = 0;

    while (totalBytesRead < dataSize) {
        size_t toRead = std::min(dataSize - totalBytesRead, buff.size());
        ssize_t bytesRead = recv(fd, buff.data(), toRead, 0);

        if (bytesRead == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return L_TCP_SOCKET_RES::BLOCKING;
            }
            return L_TCP_SOCKET_RES::UNKNOWN_ERROR;
        }

        if (bytesRead == 0) {
            return L_TCP_SOCKET_RES::CLIENT_CON_CLOSED;
        }

        dataOut.insert(dataOut.end(), buff.begin(), buff.begin() + bytesRead);
        totalBytesRead += bytesRead;
    }

    return L_TCP_SOCKET_RES::SUCCESSFUL_READ;
}

L_TCP_SOCKET_RES sendData(int fd, const byte_array& dataIn) {
    const char* pos = dataIn.data();
    size_t toSend = dataIn.size();
    size_t totalBytesSent = 0;

    while (totalBytesSent < dataIn.size()) {
        ssize_t bytesSent = send(fd, pos, toSend, 0);

        if (bytesSent == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return L_TCP_SOCKET_RES::BLOCKING;
            }
            return L_TCP_SOCKET_RES::UNKNOWN_ERROR;
        }

        if (bytesSent == 0) {
            return L_TCP_SOCKET_RES::CLIENT_CON_CLOSED;
        }

        totalBytesSent += bytesSent;
        pos += bytesSent;
        toSend -= bytesSent;
    }

    return L_TCP_SOCKET_RES::SUCCESSFUL_SEND;
}

ServerCon::ServerCon(int port, size_t maxConnections) {
    if (port <= 0) {
        throw std::runtime_error("Portnumber can't be less than 0");
    }

    port_ = port;
    maxConnections_ = maxConnections;
    init();
}

ServerCon::~ServerCon() {
    if (socket_ > 0) {
        close(socket_);
    }
}

std::optional<Client> ServerCon::acceptConnection() {
    sockaddr_in clientAddress;
    socklen_t clientSize = sizeof(clientAddress);

    int clientSocket = accept(socket_, (sockaddr*)&clientAddress, &clientSize);

    if (clientSocket < 0) {
        std::cerr << "Failed to accept incoming connection: " << strerror(errno) << '\n';
        return std::nullopt;
    }

    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) != 0) {
        std::cerr << "Failed to set socket to nonblocking: " << strerror(errno) << std::endl;
        return std::nullopt;
    }

    Client client = {
        .fd = clientSocket,
        .addr = clientAddress,
        .buff = std::string()
    };

    return client;
}

int ServerCon::getPort() const {
    return this->port_;
}

void ServerCon::init() {
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ < 0) {
        throw std::runtime_error(std::string("Failed to create socket: ") + strerror(errno));
    }

    int opt = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
        throw std::runtime_error(std::string("Failed to set socket options: ") + strerror(errno));
    }

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port_);
    addr_.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_, (const sockaddr*)&addr_, sizeof(addr_)) != 0) {
        throw std::runtime_error(std::string("Failed to bind socket: ") + strerror(errno));
    }

    if (listen(socket_, maxConnections_) != 0) {
        throw std::runtime_error(std::string("Failed to listen: ") + strerror(errno));
    }
}

}