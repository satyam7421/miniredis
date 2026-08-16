#pragma once

#include <string>
#include <system_error>
#include <cstdint>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL (-1)
#endif

namespace miniredis::network {

class Socket {
public:
    static bool init_network();
    static void cleanup_network();

    Socket();
    explicit Socket(socket_t fd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool bind(const std::string& host, uint16_t port);
    bool listen(int backlog = 128);
    Socket accept();
    bool connect(const std::string& host, uint16_t port);

    int send(const std::string& data);
    int recv(char* buffer, size_t size);

    void close();
    [[nodiscard]] bool is_valid() const { return fd_ != INVALID_SOCKET_VAL; }
    [[nodiscard]] socket_t get_fd() const { return fd_; }

private:
    socket_t fd_{INVALID_SOCKET_VAL};
};

} // namespace miniredis::network
