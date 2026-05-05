#ifndef LTALE_SOCKET_TCP_HPP_
#define LTALE_SOCKET_TCP_HPP_

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

static constexpr size_t TCP_BUFFER_SIZE = 2048;

using byte_array = std::vector<char>;

enum class L_TCP_SOCKET_RES {
    SUCCESSFUL_READ,
    SUCCESSFUL_SEND,
    BUFFER_OVERFLOW,
    BLOCKING,
    CLIENT_CON_CLOSED,
    UNKNOWN_ERROR
};

/**
 * Reads up to dataSize bytes from the socket into dataOut.
 * dataOut is cleared before reading begins.
 *
 * @return L_TCP_SOCKET_RES
 */
L_TCP_SOCKET_RES readData(int fd, byte_array& dataOut, size_t dataSize);

/**
 * Sends all of dataIn through the socket.
 *
 * @return L_TCP_SOCKET_RES
 */
L_TCP_SOCKET_RES sendData(int fd, const byte_array& dataIn);

struct Client {
    int             fd;
    sockaddr_in     addr;
    std::string     buff;
};

/**
 * Class for server-side TCP logic
 */
class ServerCon {
public:
    ServerCon(int port, size_t maxConnections);
    ~ServerCon();

    /**
     * Wraps accept() and returns a ClientCon on success.
     *
     * @return optionally returns a ClientCon
     */
    std::optional<Client> acceptConnection();

    int getPort() const;

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
