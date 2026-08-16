#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <cstdint>

namespace miniredis::storage {

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

struct Value {
    std::string data;
    std::optional<TimePoint> expires_at;

    [[nodiscard]] bool is_expired() const {
        if (!expires_at.has_value()) {
            return false;
        }
        return Clock::now() >= expires_at.value();
    }

    [[nodiscard]] int64_t ttl_seconds() const {
        if (!expires_at.has_value()) {
            return -1; // No TTL set
        }
        auto now = Clock::now();
        if (now >= expires_at.value()) {
            return -2; // Expired
        }
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(expires_at.value() - now).count();
        return (diff + 999) / 1000; // Round up so 0.9s returns 1s instead of 0s
    }

};

} // namespace miniredis::storage
