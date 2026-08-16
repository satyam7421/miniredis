#pragma once

#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <cstdint>

namespace miniredis::protocol {

enum class RespType {
    SimpleString,
    Error,
    Integer,
    BulkString,
    Array,
    Null
};

struct RespObject {
    RespType type{RespType::Null};
    std::string value{};
    int64_t integer_value{0};
    std::vector<RespObject> array_value{};

    // Helper constructor methods
    static RespObject make_simple_string(std::string str) {
        return RespObject{.type = RespType::SimpleString, .value = std::move(str)};
    }

    static RespObject make_error(std::string err) {
        return RespObject{.type = RespType::Error, .value = std::move(err)};
    }

    static RespObject make_integer(int64_t num) {
        return RespObject{.type = RespType::Integer, .integer_value = num};
    }

    static RespObject make_bulk_string(std::string str) {
        return RespObject{.type = RespType::BulkString, .value = std::move(str)};
    }

    static RespObject make_null() {
        return RespObject{.type = RespType::Null};
    }

    static RespObject make_array(std::vector<RespObject> arr) {
        return RespObject{.type = RespType::Array, .array_value = std::move(arr)};
    }
};

class RespParser {
public:
    // Serializer methods
    static std::string serialize(const RespObject& obj);
    
    // Deserializer: Parses a buffer, returns the parsed RespObject and bytes consumed
    static std::pair<RespObject, size_t> parse(const std::string& buffer);
};

} // namespace miniredis::protocol
