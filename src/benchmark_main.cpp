#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <numeric>
#include "network/socket.hpp"
#include "protocol/resp.hpp"

using namespace miniredis::network;
using namespace miniredis::protocol;

struct ClientStats {
    uint64_t total_requests{0};
    uint64_t total_errors{0};
    std::vector<double> latencies_ms;
};

void run_client_worker(const std::string& host, uint16_t port, int requests_per_client, ClientStats& stats) {
    Socket client;
    if (!client.connect(host, port)) {
        stats.total_errors += requests_per_client;
        return;
    }

    stats.latencies_ms.reserve(requests_per_client);
    char buffer[1024];

    std::string set_req = RespParser::serialize(RespObject::make_array({
        RespObject::make_bulk_string("SET"),
        RespObject::make_bulk_string("bench_key"),
        RespObject::make_bulk_string("bench_val")
    }));

    for (int i = 0; i < requests_per_client; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        if (client.send(set_req) <= 0) {
            stats.total_errors++;
            continue;
        }

        int bytes_read = client.recv(buffer, sizeof(buffer) - 1);
        auto end = std::chrono::high_resolution_clock::now();

        if (bytes_read <= 0) {
            stats.total_errors++;
            continue;
        }

        double latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
        stats.latencies_ms.push_back(latency_ms);
        stats.total_requests++;
    }

    client.close();
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;
    int num_clients = 50;
    int requests_per_client = 1000;

    if (argc >= 2) num_clients = std::stoi(argv[1]);
    if (argc >= 3) requests_per_client = std::stoi(argv[2]);
    if (argc >= 4) host = argv[3];
    if (argc >= 5) port = static_cast<uint16_t>(std::stoi(argv[4]));

    if (!Socket::init_network()) {
        std::cerr << "Failed to initialize Winsock/Sockets." << std::endl;
        return 1;
    }

    std::cout << "==========================================" << std::endl;
    std::cout << " MiniRedis High-Concurrency Benchmark Tool" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Target: " << host << ":" << port << std::endl;
    std::cout << "Concurrent Clients: " << num_clients << std::endl;
    std::cout << "Requests per Client: " << requests_per_client << std::endl;
    std::cout << "Total Requests: " << num_clients * requests_per_client << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    std::vector<ClientStats> stats_per_client(num_clients);
    std::vector<std::thread> threads;
    threads.reserve(num_clients);

    auto global_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back(run_client_worker, host, port, requests_per_client, std::ref(stats_per_client[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto global_end = std::chrono::high_resolution_clock::now();
    double total_time_sec = std::chrono::duration<double>(global_end - global_start).count();

    // Aggregate statistics
    uint64_t total_requests = 0;
    uint64_t total_errors = 0;
    std::vector<double> all_latencies;

    for (const auto& s : stats_per_client) {
        total_requests += s.total_requests;
        total_errors += s.total_errors;
        all_latencies.insert(all_latencies.end(), s.latencies_ms.begin(), s.latencies_ms.end());
    }

    std::sort(all_latencies.begin(), all_latencies.end());

    double rps = total_requests / total_time_sec;
    double avg_latency = all_latencies.empty() ? 0.0 : std::accumulate(all_latencies.begin(), all_latencies.end(), 0.0) / all_latencies.size();
    double p50 = all_latencies.empty() ? 0.0 : all_latencies[all_latencies.size() * 50 / 100];
    double p95 = all_latencies.empty() ? 0.0 : all_latencies[all_latencies.size() * 95 / 100];
    double p99 = all_latencies.empty() ? 0.0 : all_latencies[all_latencies.size() * 99 / 100];

    std::cout << "RESULTS:" << std::endl;
    std::cout << "  Time Taken:     " << total_time_sec << " seconds" << std::endl;
    std::cout << "  Successful Req: " << total_requests << std::endl;
    std::cout << "  Failed Req:     " << total_errors << std::endl;
    std::cout << "  Throughput:     " << rps << " req/sec (RPS)" << std::endl;
    std::cout << "  Avg Latency:    " << avg_latency << " ms" << std::endl;
    std::cout << "  p50 Latency:    " << p50 << " ms" << std::endl;
    std::cout << "  p95 Latency:    " << p95 << " ms" << std::endl;
    std::cout << "  p99 Latency:    " << p99 << " ms" << std::endl;
    std::cout << "==========================================" << std::endl;

    Socket::cleanup_network();
    return 0;
}
