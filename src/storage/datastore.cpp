#include "storage/datastore.hpp"
#include <mutex>
#include <algorithm>
#include <random>

namespace miniredis::storage {

void DataStore::set(const std::string& key, std::string value, std::optional<int64_t> ttl_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Value val{.data = std::move(value), .expires_at = std::nullopt};
    if (ttl_seconds.has_value() && ttl_seconds.value() > 0) {
        val.expires_at = Clock::now() + std::chrono::seconds(ttl_seconds.value());
    }
    store_[key] = std::move(val);
}

std::optional<std::string> DataStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return std::nullopt;
    }
    if (it->second.is_expired()) {
        store_.erase(it);
        return std::nullopt;
    }
    return it->second.data;
}

bool DataStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.erase(key) > 0;
}

bool DataStore::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return false;
    }
    if (it->second.is_expired()) {
        store_.erase(it);
        return false;
    }
    return true;
}

int64_t DataStore::ttl(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return -2; // Key does not exist
    }
    if (it->second.is_expired()) {
        store_.erase(it);
        return -2;
    }
    return it->second.ttl_seconds();
}

size_t DataStore::dbsize() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& [_, val] : store_) {
        if (!val.is_expired()) {
            count++;
        }
    }
    return count;
}

void DataStore::flushdb() {
    std::lock_guard<std::mutex> lock(mutex_);
    store_.clear();
}

std::vector<std::string> DataStore::keys() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(store_.size());
    for (const auto& [k, val] : store_) {
        if (!val.is_expired()) {
            result.push_back(k);
        }
    }
    return result;
}

size_t DataStore::expire_sample(size_t sample_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.empty()) return 0;

    size_t expired_count = 0;
    size_t checked = 0;
    auto now = Clock::now();

    auto it = store_.begin();
    while (it != store_.end() && checked < sample_size) {
        checked++;
        if (it->second.expires_at.has_value() && now >= it->second.expires_at.value()) {
            it = store_.erase(it);
            expired_count++;
        } else {
            ++it;
        }
    }
    return expired_count;
}


} // namespace miniredis::storage
