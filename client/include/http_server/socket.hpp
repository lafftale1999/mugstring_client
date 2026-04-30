#ifndef LTALE_SOCKET_TCP_HPP_
#define LTALE_SOCKET_TCP_HPP_

#include <iostream>
#include <vector>
#include <optional>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace networking::tcp {

#define L_TCP_SOCKET_BUFFER_SIZE        2048

using byte_array = std::vector<char>;

class SocketBase {
public:
    SocketBase() = default;
    virtual ~SocketBase() = 0;

    int getPort() const; 
    void setPort(int port);

protected:
    int             socket_;
    sockaddr_in     addr_;
    int             port_;
};

class ClientSocket : public SocketBase {
public:
    ClientSocket(int socket, sockaddr_in addr);
    virtual ~ClientSocket() override;

    uint8_t readData(byte_array& dataOut, size_t dataSize);
    uint8_t sendData(const byte_array& dataIn);
    uint8_t poll();

    void setIP(char ip[INET_ADDRSTRLEN]);
    const char* getIP() const;

private:
    char            ip_[INET_ADDRSTRLEN];
    int             timeout_;
};

class ServerSocket : public SocketBase {
public:
    ServerSocket(int port);
    virtual ~ServerSocket() override;

    std::optional<std::vector<ClientSocket>> poll();

private:
    uint8_t init();
};
}

#endif