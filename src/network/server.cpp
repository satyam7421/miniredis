#include "network/server.hpp"
#include "protocol/resp.hpp"
#include "commands/command.hpp"
#include <iostream>
#include <vector>

namespace miniredis::network {

Server::Server(std::string host, uint16_t port, size_t worker_threads)
    : host_(std::move(host)), port_(port), thread_pool_(worker_threads), aof_manager_("miniredis.aof") {
    Socket::init_network();
    server_socket_ = Socket();
}


Server::~Server() {
    stop();
}

bool Server::start() {
    // Phase 5 Recovery: Replay existing AOF state
    size_t replayed = aof_manager_.load_and_replay(store_);
    if (replayed > 0) {
        std::cout << "[AOF] Successfully replayed " << replayed << " command(s) from disk." << std::endl;
    }

    if (!server_socket_.bind(host_, port_)) {
        std::cerr << "Failed to bind socket to " << host_ << ":" << port_ << std::endl;
        return false;
    }



    if (!server_socket_.listen(128)) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "MiniRedis Server listening on " << host_ << ":" << port_ << std::endl;

    while (running_) {
        Socket client = server_socket_.accept();
        if (!client.is_valid()) {
            if (!running_) break;
            continue;
        }

        // Offload client connection handling to ThreadPool
        thread_pool_.enqueue([this, client_socket = std::move(client)]() mutable {
            handle_client(std::move(client_socket));
        });
    }

    return true;
}

void Server::stop() {
    if (running_) {
        running_ = false;
        server_socket_.close();
        Socket::cleanup_network();
    }
}

void Server::handle_client(Socket client_socket) {
    constexpr size_t buffer_size = 4096;
    std::vector<char> buffer(buffer_size);
    std::string client_buffer;

    while (running_ && client_socket.is_valid()) {
        int bytes_read = client_socket.recv(buffer.data(), buffer.size());
        if (bytes_read <= 0) {
            break; // Client disconnected or error
        }

        client_buffer.append(buffer.data(), static_cast<size_t>(bytes_read));

        while (!client_buffer.empty()) {
            auto [resp_obj, bytes_consumed] = protocol::RespParser::parse(client_buffer);
            if (bytes_consumed == 0) {
                break; // Need more data over TCP socket
            }

            client_buffer.erase(0, bytes_consumed);

            // Convert RESP object to Command parameters
            std::vector<std::string> args;
            if (resp_obj.type == protocol::RespType::Array) {
                for (const auto& elem : resp_obj.array_value) {
                    args.push_back(elem.value);
                }
            } else if (resp_obj.type == protocol::RespType::BulkString || resp_obj.type == protocol::RespType::SimpleString) {
                args.push_back(resp_obj.value);
            }

            if (args.empty()) continue;

            auto cmd = commands::CommandRegistry::parse(args);
            std::string response = cmd->execute(store_);

            // If command modifies state, append to AOF log
            std::string upper_cmd = args[0];
            for (auto& c : upper_cmd) c = static_cast<char>(toupper(c));
            if (upper_cmd == "SET" || upper_cmd == "DEL" || upper_cmd == "FLUSHDB") {
                aof_manager_.append(protocol::RespParser::serialize(resp_obj));
            }

            client_socket.send(response);
        }
    }
}

} // namespace miniredis::network
