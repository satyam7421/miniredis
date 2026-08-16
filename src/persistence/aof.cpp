#include "persistence/aof.hpp"
#include "protocol/resp.hpp"
#include "commands/command.hpp"
#include <iostream>
#include <sstream>

namespace miniredis::persistence {

AofManager::AofManager(std::string filename) : filename_(std::move(filename)) {
    file_.open(filename_, std::ios::out | std::ios::app | std::ios::binary);
}

AofManager::~AofManager() {
    close();
}

void AofManager::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.close();
    }
}


void AofManager::append(const std::string& resp_cmd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.write(resp_cmd.data(), resp_cmd.size());
        file_.flush();
    }
}

size_t AofManager::load_and_replay(storage::DataStore& store) {
    std::ifstream in(filename_, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return 0;
    }

    std::string buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    size_t replayed_count = 0;
    size_t offset = 0;

    while (offset < buffer.length()) {
        auto [resp_obj, bytes_consumed] = protocol::RespParser::parse(buffer.substr(offset));
        if (bytes_consumed == 0) {
            break;
        }

        offset += bytes_consumed;

        std::vector<std::string> args;
        if (resp_obj.type == protocol::RespType::Array) {
            for (const auto& elem : resp_obj.array_value) {
                args.push_back(elem.value);
            }
        } else if (resp_obj.type == protocol::RespType::BulkString || resp_obj.type == protocol::RespType::SimpleString) {
            args.push_back(resp_obj.value);
        }

        if (!args.empty()) {
            auto cmd = commands::CommandRegistry::parse(args);
            cmd->execute(store);
            replayed_count++;
        }
    }

    return replayed_count;
}

bool SnapshotManager::save(storage::DataStore& store, const std::string& filename) {
    std::ofstream out(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;

    // Header magic bytes "MINIREDIS"
    out.write("MINIREDIS", 9);

    std::vector<std::string> all_keys = store.keys();
    uint64_t num_keys = all_keys.size();
    out.write(reinterpret_cast<const char*>(&num_keys), sizeof(num_keys));

    for (const auto& k : all_keys) {
        auto val = store.get(k);
        if (!val.has_value()) continue;

        uint32_t key_len = static_cast<uint32_t>(k.length());
        uint32_t val_len = static_cast<uint32_t>(val->length());

        out.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        out.write(k.data(), key_len);

        out.write(reinterpret_cast<const char*>(&val_len), sizeof(val_len));
        out.write(val->data(), val_len);
    }

    out.close();
    return true;
}

bool SnapshotManager::load(storage::DataStore& store, const std::string& filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in.is_open()) return false;

    char header[9];
    in.read(header, 9);
    if (std::string(header, 9) != "MINIREDIS") {
        return false;
    }

    uint64_t num_keys = 0;
    in.read(reinterpret_cast<char*>(&num_keys), sizeof(num_keys));

    for (uint64_t i = 0; i < num_keys; ++i) {
        uint32_t key_len = 0;
        in.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        std::string key(key_len, '\0');
        in.read(&key[0], key_len);

        uint32_t val_len = 0;
        in.read(reinterpret_cast<char*>(&val_len), sizeof(val_len));
        std::string val(val_len, '\0');
        in.read(&val[0], val_len);

        store.set(key, val);
    }

    in.close();
    return true;
}

} // namespace miniredis::persistence
