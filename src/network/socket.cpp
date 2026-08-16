#include "network/socket.hpp"
#include <iostream>
#include <utility>

namespace miniredis::network {

bool Socket::init_network() {
#ifdef _WIN32
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cerr << "WSAStartup failed: " << res << std::endl;
        return false;
    }
#endif
    return true;
}

void Socket::cleanup_network() {
#ifdef _WIN32
    WSACleanup();
#endif
}

Socket::Socket() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_ == INVALID_SOCKET_VAL) {
        std::cerr << "Socket creation failed" << std::endl;
    }
    
    int opt = 1;
#ifdef _WIN32
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
}

Socket::Socket(socket_t fd) : fd_(fd) {}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = INVALID_SOCKET_VAL;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (is_valid()) {
            close();
        }
        fd_ = other.fd_;
        other.fd_ = INVALID_SOCKET_VAL;
    }
    return *this;
}


bool Socket::bind(const std::string& host, uint16_t port) {
    if (!is_valid()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    int res = ::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    return res != SOCKET_ERROR_VAL;
}

bool Socket::listen(int backlog) {
    if (!is_valid()) return false;
    return ::listen(fd_, backlog) != SOCKET_ERROR_VAL;
}

Socket Socket::accept() {
    if (!is_valid()) return Socket(INVALID_SOCKET_VAL);

    sockaddr_in client_addr{};
#ifdef _WIN32
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    socket_t client_fd = ::accept(fd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    return Socket(client_fd);
}

bool Socket::connect(const std::string& host, uint16_t port) {
    if (!is_valid()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    return ::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != SOCKET_ERROR_VAL;
}

int Socket::send(const std::string& data) {
    if (!is_valid()) return -1;
#ifdef _WIN32
    return ::send(fd_, data.c_str(), static_cast<int>(data.length()), 0);
#else
    return static_cast<int>(::send(fd_, data.c_str(), data.length(), 0));
#endif
}

int Socket::recv(char* buffer, size_t size) {
    if (!is_valid()) return -1;
#ifdef _WIN32
    return ::recv(fd_, buffer, static_cast<int>(size), 0);
#else
    return static_cast<int>(::recv(fd_, buffer, size, 0));
#endif
}

void Socket::close() {
    if (fd_ != INVALID_SOCKET_VAL) {
#ifdef _WIN32
        ::closesocket(fd_);
#else
        ::close(fd_);
#endif
        fd_ = INVALID_SOCKET_VAL;
    }
}

} // namespace miniredis::network
