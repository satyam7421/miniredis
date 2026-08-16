#pragma once

#include "storage/datastore.hpp"
#include <string>
#include <vector>
#include <memory>

namespace miniredis::commands {

class Command {
public:
    virtual ~Command() = default;
    [[nodiscard]] virtual std::string execute(storage::DataStore& store) = 0;
};

class CommandRegistry {
public:
    static std::unique_ptr<Command> parse(const std::vector<std::string>& args);
};

// Core Commands
class SetCommand : public Command {
public:
    SetCommand(std::string key, std::string value, std::optional<int64_t> ttl = std::nullopt)
        : key_(std::move(key)), value_(std::move(value)), ttl_(ttl) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::string key_;
    std::string value_;
    std::optional<int64_t> ttl_;
};

class GetCommand : public Command {
public:
    explicit GetCommand(std::string key) : key_(std::move(key)) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::string key_;
};

class DelCommand : public Command {
public:
    explicit DelCommand(std::vector<std::string> keys) : keys_(std::move(keys)) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::vector<std::string> keys_;
};

class ExistsCommand : public Command {
public:
    explicit ExistsCommand(std::vector<std::string> keys) : keys_(std::move(keys)) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::vector<std::string> keys_;
};

class TtlCommand : public Command {
public:
    explicit TtlCommand(std::string key) : key_(std::move(key)) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::string key_;
};

class DbsizeCommand : public Command {
public:
    std::string execute(storage::DataStore& store) override;
};

class FlushDbCommand : public Command {
public:
    std::string execute(storage::DataStore& store) override;
};

class UnknownCommand : public Command {
public:
    explicit UnknownCommand(std::string name) : name_(std::move(name)) {}
    std::string execute(storage::DataStore& store) override;
private:
    std::string name_;
};

} // namespace miniredis::commands
