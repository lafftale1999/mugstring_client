#ifndef LTALE_SOCKET_TCP_HPP_
#define LTALE_SOCKET_TCP_HPP_

#include <vector>
#include <optional>
#include <cerrno>
#include <cstring>
#include <string>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

namespace networking::http::tcp {

static constexpr size_t TCP_BUFFER_SIZE = 2048;

using byte_array = std::vector<char>;

enum class L_TCP_SOCKET_RES {
    SUCCESSFUL_READ,
    SUCCESSFUL_SEND,
    BLOCKING,
    CLIENT_CON_CLOSED,
    UNKNOWN_ERROR
};

/**
 * Reads up to dataSize bytes from the socket into dataOut.
 * dataOut is cleared before reading begins.
 *
 * @param fd socket file descriptor
 * @param dataOut byte_array to add data in
 * @param dataSize maximum size of dataOut
 * 
 * @return L_TCP_SOCKET_RES
 */
L_TCP_SOCKET_RES readData(int fd, byte_array& dataOut, size_t dataSize);

/**
 * Sends all of dataIn through the socket.
 *
 * @param fd socket file descriptor
 * @param dataIn byte_array of data to be sent
 * @return L_TCP_SOCKET_RES
 */
L_TCP_SOCKET_RES sendData(int fd, const byte_array& dataIn);

struct Client {
    int             fd;
    sockaddr_in     addr;
    byte_array      buff;
};

/**
 * Class for server-side TCP logic
 */
class ServerCon {
public:
    ServerCon(int port, size_t maxConnections);
    ~ServerCon();

    /**
     * Wraps accept() and returns a Client on success.
     *
     * @return optionally returns a Client
     */
    std::optional<Client> acceptConnection();

    int getPort() const;
    int getSocket() const;

private:
    /** Initializes the server socket. Throws std::runtime_error on failure. */
    void init();
    
    int             socket_;
    sockaddr_in     addr_;
    int             port_;
    size_t          maxConnections_;
};
}

#endif
