#include <gtest/gtest.h>
#include "network/socket.hpp"
#include "network/server.hpp"
#include "protocol/resp.hpp"

#include <thread>
#include <chrono>

using namespace miniredis::network;
using namespace miniredis::protocol;

TEST(NetworkTest, ServerClientCommunication) {
    std::string host = "127.0.0.1";
    uint16_t port = 6389; // Dedicated test port

    // Launch server in background thread
    std::thread server_thread([host, port]() {
        Server server(host, port, 2);
        // Stop server after 2 seconds
        std::thread stopper([&server]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            server.stop();
        });
        stopper.detach();
        server.start();
    });

    // Wait for server to bind & listen
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Client connection
    ASSERT_TRUE(Socket::init_network());
    Socket client;
    ASSERT_TRUE(client.connect(host, port));

    // Send SET command
    std::string set_req = "*3\r\n$3\r\nSET\r\n$4\r\nuser\r\n$6\r\nSatyam\r\n";
    EXPECT_GT(client.send(set_req), 0);

    char buffer[1024];
    int read_bytes = client.recv(buffer, sizeof(buffer) - 1);
    ASSERT_GT(read_bytes, 0);
    buffer[read_bytes] = '\0';
    EXPECT_STREQ(buffer, "+OK\r\n");

    // Send GET command
    std::string get_req = "*2\r\n$3\r\nGET\r\n$4\r\nuser\r\n";
    EXPECT_GT(client.send(get_req), 0);

    read_bytes = client.recv(buffer, sizeof(buffer) - 1);
    ASSERT_GT(read_bytes, 0);
    buffer[read_bytes] = '\0';
    EXPECT_STREQ(buffer, "$6\r\nSatyam\r\n");

    client.close();
    if (server_thread.joinable()) {
        server_thread.join();
    }
}
