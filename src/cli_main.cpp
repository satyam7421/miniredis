#include <iostream>
#include <string>
#include <vector>
#include "network/socket.hpp"
#include "protocol/resp.hpp"

using namespace miniredis::network;
using namespace miniredis::protocol;

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = static_cast<uint16_t>(std::stoi(argv[2]));

    if (!Socket::init_network()) {
        std::cerr << "Failed to initialize Winsock/Sockets." << std::endl;
        return 1;
    }

    Socket client;
    if (!client.connect(host, port)) {
        std::cerr << "Could not connect to MiniRedis at " << host << ":" << port << std::endl;
        Socket::cleanup_network();
        return 1;
    }

    std::cout << "Connected to MiniRedis at " << host << ":" << port << std::endl;
    std::cout << "Type commands (e.g. 'SET name Satyam', 'GET name', 'exit' to quit):" << std::endl;

    std::string line;
    char buffer[4096];

    while (true) {
        std::cout << host << ":" << port << "> ";
        if (!std::getline(std::cin, line) || line == "exit" || line == "quit") {
            break;
        }

        if (line.empty()) continue;

        // Parse line into inline RESP Array
        std::string req = line + "\r\n";
        auto [resp_obj, _] = RespParser::parse(req);
        std::string serialized_req = RespParser::serialize(resp_obj);

        if (client.send(serialized_req) <= 0) {
            std::cerr << "Server disconnected." << std::endl;
            break;
        }

        int bytes_read = client.recv(buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cerr << "Server closed connection." << std::endl;
            break;
        }

        buffer[bytes_read] = '\0';
        std::string raw_res(buffer, bytes_read);

        // Format RESP output for CLI
        auto [res_obj, __] = RespParser::parse(raw_res);
        if (res_obj.type == RespType::SimpleString) {
            std::cout << res_obj.value << std::endl;
        } else if (res_obj.type == RespType::Error) {
            std::cout << "(error) " << res_obj.value << std::endl;
        } else if (res_obj.type == RespType::Integer) {
            std::cout << "(integer) " << res_obj.integer_value << std::endl;
        } else if (res_obj.type == RespType::BulkString) {
            std::cout << "\"" << res_obj.value << "\"" << std::endl;
        } else if (res_obj.type == RespType::Null) {
            std::cout << "(nil)" << std::endl;
        } else {
            std::cout << raw_res;
        }
    }

    client.close();
    Socket::cleanup_network();
    return 0;
}
