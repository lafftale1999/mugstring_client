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

#define L_TCP_SOCKET_BUFFER_SIZE        2048    /**< Maximum reading size for tcp socket */

using byte_array = std::vector<char>;

enum class L_TCP_SOCKET_RES {
    SUCCESSFUL_READ,
    SUCCESSFUL_SEND,
    BUFFER_OVERFLOW,
    BLOCKING,
    CLIENT_CON_CLOSED,
    UNKOWN_ERROR  
};

/**
 * Virtual base class representing a TCP-connection
 */
class ConBase {
public:
    ConBase() = default;
    virtual ~ConBase() = 0;

    int getPort() const; 
    void setPort(int port);

protected:
    int             socket_;
    sockaddr_in     addr_;
    int             port_;
};

/**
 * Class representing a Client connection over TCP. Only accepts non-blocking connections
 */
class ClientCon : public ConBase {
public:
    /**
     * Constructing the ConBase class.
     * 
     * @param socket Filedescriptor for the socket, this socket need to be declared as
     * non-blocking.
     * 
     * @param addr Already Initialized sockaddr_in
     */
    ClientCon(int socket, sockaddr_in addr);
    virtual ~ClientCon() override;

    /**
     * Reads data from the socket until it is maxed out in dataSize or if the socket
     * has nothing more to read.
     * 
     * @param dataOut A char array to stream bytes from,
     * @param dataSize A set size of the data buffer to easily restrict how much to read.
     * 
     * @return L_TCP_SOCKET_RES
     */
    L_TCP_SOCKET_RES readData(byte_array& dataOut, size_t dataSize);

    /**
     * Sends the data through the specified socket.
     * 
     * @param dataIn The data to send on the socket.
     * 
     * @return L_TCP_SOCKET_RES
     */
    L_TCP_SOCKET_RES sendData(const byte_array& dataIn);

    void setIP();
    const char* getIP() const;

private:
    char            ip_[INET_ADDRSTRLEN];
    int             timeout_;
};

class ServerCon : public ConBase {
public:
    ServerCon(int port);
    virtual ~ServerCon() override;

    /**
     * Polls the server socket until it returns a blocking signal.
     * 
     * @return optionally a vector of initialized `ClientCon`
     */
    std::optional<std::vector<ClientCon>> poll();

private:
    uint8_t init();
};
}

#endif