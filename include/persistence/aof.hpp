#pragma once

#include "storage/datastore.hpp"
#include <string>
#include <fstream>
#include <mutex>
#include <vector>

namespace miniredis::persistence {

class AofManager {
public:
    explicit AofManager(std::string filename = "miniredis.aof");
    ~AofManager();

    AofManager(const AofManager&) = delete;
    AofManager& operator=(const AofManager&) = delete;

    void append(const std::string& resp_cmd);
    size_t load_and_replay(storage::DataStore& store);
    void close();

private:
    std::string filename_;

    std::ofstream file_;
    std::mutex mutex_;
};

class SnapshotManager {
public:
    static bool save(storage::DataStore& store, const std::string& filename = "dump.rdb");
    static bool load(storage::DataStore& store, const std::string& filename = "dump.rdb");
};

} // namespace miniredis::persistence
