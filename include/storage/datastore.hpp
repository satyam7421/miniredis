#pragma once

#include "storage/value.hpp"

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <vector>
#include <cstdint>

namespace miniredis::storage {

class DataStore {
public:
    DataStore() = default;
    ~DataStore() = default;

    // Prevent copying
    DataStore(const DataStore&) = delete;
    DataStore& operator=(const DataStore&) = delete;

    // Core Operations
    void set(const std::string& key, std::string value, std::optional<int64_t> ttl_seconds = std::nullopt);
    [[nodiscard]] std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    [[nodiscard]] bool exists(const std::string& key);
    [[nodiscard]] int64_t ttl(const std::string& key);
    
    // Administrative Operations
    [[nodiscard]] size_t dbsize();
    void flushdb();
    [[nodiscard]] std::vector<std::string> keys();

    // Active TTL Expiration Task (returns count of expired keys removed)
    size_t expire_sample(size_t sample_size = 20);

private:
    std::unordered_map<std::string, Value> store_;
    mutable std::mutex mutex_;
};


} // namespace miniredis::storage
