#include "socket.hpp"
#include <array>
#include <fcntl.h>
#include <iterator>

namespace networking::tcp {

/**
 * ------------ SOCKET BASE ------------
 */

SocketBase::~SocketBase() {
    if (this->socket_ > 0) {
        close(this->socket_);
    }
}

int SocketBase::getPort() const {
    return this->port_;
}

void SocketBase::setPort(int port) {
    if (port <= 0) {
        throw std::invalid_argument("Port number cant be less than or equal to 0");
    }

    port_ = port;
}

/**
 * ------------ CLIENT SOCKET ------------
 */

ClientSocket::ClientSocket(int s, sockaddr_in addr) {
    if (socket <= 0) {
        throw std::runtime_error("Socket is not valid");
    }

    this->socket_ = s;
    this->addr_ = addr;

    if (fcntl(this->socket_, F_SETFL, O_NONBLOCK) < 0) {
        throw std::runtime_error("Failed to set socket to non-blocking");
    }
}

ClientSocket::~ClientSocket() {}

uint8_t ClientSocket::readData(byte_array& dataOut, size_t dataSize) {
    std::array<char, L_TCP_SOCKET_BUFFER_SIZE> buff;
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < dataSize) {
        std::memset(buff.data(), 0, buff.size());

        ssize_t bytesRead = recv(this->socket_, buff.data(), buff.size(), 0);

        if ((bytesRead + totalBytesRead) < dataSize) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return EWOULDBLOCK;
            } else if (bytesRead > 0) {
                dataOut.insert(
                    dataOut.end(), 
                    std::make_move_iterator(buff.begin()),
                    std::make_move_iterator(buff.begin() + bytesRead)
                );
                totalBytesRead += bytesRead;
            } else if (bytesRead == 0) {
                return 0;
            } else {
                throw std::runtime_error("Socket entered unexpected state");
            }
        } else {
            return 0;
        }
    }
}

uint8_t ClientSocket::sendData(const byte_array& dataIn) {
    size_t toSend = dataIn.size();
    size_t totalBytesSent = 0;

    char* pos = (char*)dataIn.data();

    while (totalBytesSent < dataIn.size()) {
        ssize_t bytesSent = send(this->socket_, pos, toSend, 0);
        
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return EWOULDBLOCK;
        } else if (bytesSent > 0) {
            totalBytesSent += bytesSent;
            pos += bytesSent;
        } else if (bytesSent == 0) {
            return 0;
        } else {
            throw std::runtime_error("Socket entered unexpected state");
        }
    }

    return 0;
}

void ClientSocket::setIP(char ip[INET_ADDRSTRLEN]) {

}

const char* ClientSocket::getIP() const {

}

uint8_t ClientSocket::poll() {

}

class ServerSocket : public SocketBase {
public:
    ServerSocket(int port);
    virtual ~ServerSocket() override;

    std::optional<std::vector<ClientSocket>> poll();

private:
    uint8_t init();
};
}