#ifndef LTALE_SOCKET_TCP_HPP_
#define LTALE_SOCKET_TCP_HPP_

#include <iostream>
#include <vector>
#include <optional>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

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

/**
 * Class for server based logic
 */
class ServerCon : public ConBase {
public:
    ServerCon(int port, size_t maxConnections);
    virtual ~ServerCon() override;

    /**
     * Wraps the accept() function and returns a instance of the ClientCon.
     * 
     * @return optionally returns a ClientCon when read properly
     */
    std::optional<ClientCon> acceptConnection();

private:

    /**
     * Initializes the ServerCon
     * 
     * @return 0 on success, 1 on failure.
     */
    uint8_t init();

    size_t maxConnections_;
};
}

#endif