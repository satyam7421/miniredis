#include <iostream>
#include <csignal>
#include "network/server.hpp"

static miniredis::network::Server* g_server = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT && g_server) {
        std::cout << "\nShutting down MiniRedis server..." << std::endl;
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = static_cast<uint16_t>(std::stoi(argv[2]));

    miniredis::network::Server server(host, port, 4);
    g_server = &server;

    std::signal(SIGINT, signal_handler);

    std::cout << "Starting MiniRedis Server v1.0.0 (TCP Network Enabled)" << std::endl;
    server.start();

    return 0;
}

