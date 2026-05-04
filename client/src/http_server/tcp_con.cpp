#include "../../include/http_server/tcp_con.hpp"
#include <array>
#include <fcntl.h>
#include <iterator>

namespace networking::tcp {

/**
 * ------------ SOCKET BASE ------------
 */

ConBase::~ConBase() {
    if (this->socket_ > 0) {
        close(this->socket_);
    }
}

int ConBase::getPort() const {
    return this->port_;
}

void ConBase::setPort(int port) {
    if (port <= 0) {
        throw std::invalid_argument("Port number cant be less than or equal to 0");
    }

    port_ = port;
}

/**
 * ------------ CLIENT SOCKET ------------
 */

ClientCon::ClientCon(int s, sockaddr_in addr) {
    if (socket <= 0) {
        throw std::runtime_error("Socket is not valid");
    }

    this->socket_ = s;
    this->addr_ = addr;
    this->setIP();
    this->setPort(ntohs(addr_.sin_port));

    if (fcntl(this->socket_, F_SETFL, O_NONBLOCK) < 0) {
        throw std::runtime_error("Failed to set socket to non-blocking");
    }
}

ClientCon::~ClientCon() {}

L_TCP_SOCKET_RES ClientCon::readData(byte_array& dataOut, size_t dataSize) {
    std::array<char, L_TCP_SOCKET_BUFFER_SIZE> buff;
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < dataSize) {
        std::memset(buff.data(), 0, buff.size());

        ssize_t bytesRead = recv(this->socket_, buff.data(), buff.size(), 0);

        if (bytesRead == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return L_TCP_SOCKET_RES::BLOCKING;
            }
            return L_TCP_SOCKET_RES::UNKOWN_ERROR;
            
        }

        if ((bytesRead + totalBytesRead) < dataSize) {
            if (bytesRead > 0) {
                dataOut.insert(
                    dataOut.end(), 
                    std::make_move_iterator(buff.begin()),
                    std::make_move_iterator(buff.begin() + bytesRead)
                );
                totalBytesRead += bytesRead;
            } else if (bytesRead == 0) {
                return L_TCP_SOCKET_RES::CLIENT_CON_CLOSED;
            } else {
                throw std::runtime_error("Socket entered unexpected state");
            }
        } else {
            return L_TCP_SOCKET_RES::BUFFER_OVERFLOW;
        }
    }

    return L_TCP_SOCKET_RES::SUCCESSFUL_READ;
}

L_TCP_SOCKET_RES ClientCon::sendData(const byte_array& dataIn) {
    size_t toSend = dataIn.size();
    size_t totalBytesSent = 0;

    char* pos = (char*)dataIn.data();

    while (totalBytesSent < dataIn.size()) {
        ssize_t bytesSent = send(this->socket_, pos, toSend, 0);
        
        if (bytesSent == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return L_TCP_SOCKET_RES::BLOCKING;
            }

            return L_TCP_SOCKET_RES::UNKOWN_ERROR;
        }

        if (bytesSent > 0) {
            totalBytesSent += bytesSent;
            pos += bytesSent;
        } else if (bytesSent == 0) {
            return L_TCP_SOCKET_RES::CLIENT_CON_CLOSED;
        } else {
            throw std::runtime_error("Socket entered unexpected state");
        }
    }

    return L_TCP_SOCKET_RES::SUCCESSFUL_SEND;
}

void ClientCon::setIP() {    
    inet_ntop(AF_INET, &addr_.sin_addr, ip_, INET_ADDRSTRLEN);
}

const char* ClientCon::getIP() const {
    return this->ip_;
}

/**
 * ------------ SERVER SOCKET ------------
 */

ServerCon::ServerCon(int port) {
    if (port <= 0) {
        throw std::runtime_error("Invalid port numbers");
    }

    this->port_ = port;
}

ServerCon::~ServerCon() {}

std::optional<std::vector<ClientCon>> ServerCon::poll() {
    std::vector<ClientCon> tmp_con;

    // poll server socket
    // append client connections
}

uint8_t ServerCon::init() {

}

}