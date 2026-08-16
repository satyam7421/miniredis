#pragma once

#include "network/socket.hpp"
#include "storage/datastore.hpp"
#include "concurrency/thread_pool.hpp"
#include "persistence/aof.hpp"

#include <string>
#include <atomic>

namespace miniredis::network {

class Server {
public:
    Server(std::string host, uint16_t port, size_t worker_threads = 4);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool start();
    void stop();

private:
    void handle_client(Socket client_socket);

    std::string host_;
    uint16_t port_;
    Socket server_socket_;
    storage::DataStore store_;
    concurrency::ThreadPool thread_pool_;
    persistence::AofManager aof_manager_;
    std::atomic<bool> running_{false};
};

} // namespace miniredis::network
