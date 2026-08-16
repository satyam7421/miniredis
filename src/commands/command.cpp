#include "commands/command.hpp"
#include <algorithm>
#include <cctype>

namespace miniredis::commands {

static std::string to_upper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
    return str;
}

std::unique_ptr<Command> CommandRegistry::parse(const std::vector<std::string>& args) {
    if (args.empty()) {
        return std::make_unique<UnknownCommand>("");
    }

    std::string cmd = to_upper(args[0]);

    if (cmd == "SET" && args.size() >= 3) {
        std::optional<int64_t> ttl = std::nullopt;
        if (args.size() >= 5 && to_upper(args[3]) == "EX") {
            try {
                ttl = std::stoll(args[4]);
            } catch (...) {}
        }
        return std::make_unique<SetCommand>(args[1], args[2], ttl);
    }

    if (cmd == "GET" && args.size() == 2) {
        return std::make_unique<GetCommand>(args[1]);
    }

    if (cmd == "DEL" && args.size() >= 2) {
        std::vector<std::string> keys(args.begin() + 1, args.end());
        return std::make_unique<DelCommand>(std::move(keys));
    }

    if (cmd == "EXISTS" && args.size() >= 2) {
        std::vector<std::string> keys(args.begin() + 1, args.end());
        return std::make_unique<ExistsCommand>(std::move(keys));
    }

    if (cmd == "TTL" && args.size() == 2) {
        return std::make_unique<TtlCommand>(args[1]);
    }

    if (cmd == "DBSIZE") {
        return std::make_unique<DbsizeCommand>();
    }

    if (cmd == "FLUSHDB") {
        return std::make_unique<FlushDbCommand>();
    }

    return std::make_unique<UnknownCommand>(args[0]);
}

std::string SetCommand::execute(storage::DataStore& store) {
    store.set(key_, value_, ttl_);
    return "+OK\r\n";
}

std::string GetCommand::execute(storage::DataStore& store) {
    auto val = store.get(key_);
    if (!val.has_value()) {
        return "$-1\r\n"; // RESP null bulk string
    }
    return "$" + std::to_string(val->length()) + "\r\n" + *val + "\r\n";
}

std::string DelCommand::execute(storage::DataStore& store) {
    int deleted = 0;
    for (const auto& k : keys_) {
        if (store.del(k)) deleted++;
    }
    return ":" + std::to_string(deleted) + "\r\n";
}

std::string ExistsCommand::execute(storage::DataStore& store) {
    int count = 0;
    for (const auto& k : keys_) {
        if (store.exists(k)) count++;
    }
    return ":" + std::to_string(count) + "\r\n";
}

std::string TtlCommand::execute(storage::DataStore& store) {
    int64_t t = store.ttl(key_);
    return ":" + std::to_string(t) + "\r\n";
}

std::string DbsizeCommand::execute(storage::DataStore& store) {
    return ":" + std::to_string(store.dbsize()) + "\r\n";
}

std::string FlushDbCommand::execute(storage::DataStore& store) {
    store.flushdb();
    return "+OK\r\n";
}

std::string UnknownCommand::execute(storage::DataStore&) {
    return "-ERR unknown command '" + name_ + "'\r\n";
}

} // namespace miniredis::commands
