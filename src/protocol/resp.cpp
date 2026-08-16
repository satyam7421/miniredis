#include "protocol/resp.hpp"
#include <sstream>
#include <iostream>

namespace miniredis::protocol {

std::string RespParser::serialize(const RespObject& obj) {
    switch (obj.type) {
        case RespType::SimpleString:
            return "+" + obj.value + "\r\n";
        case RespType::Error:
            return "-" + obj.value + "\r\n";
        case RespType::Integer:
            return ":" + std::to_string(obj.integer_value) + "\r\n";
        case RespType::BulkString:
            return "$" + std::to_string(obj.value.length()) + "\r\n" + obj.value + "\r\n";
        case RespType::Null:
            return "$-1\r\n";
        case RespType::Array: {
            std::string res = "*" + std::to_string(obj.array_value.size()) + "\r\n";
            for (const auto& elem : obj.array_value) {
                res += serialize(elem);
            }
            return res;
        }
    }
    return "-ERR serialization failed\r\n";
}

std::pair<RespObject, size_t> RespParser::parse(const std::string& buffer) {
    if (buffer.empty()) {
        return {RespObject::make_null(), 0};
    }

    char prefix = buffer[0];
    size_t crlf = buffer.find("\r\n");
    if (crlf == std::string::npos) {
        return {RespObject::make_null(), 0}; // Incomplete line
    }

    if (prefix == '+') {
        std::string line = buffer.substr(1, crlf - 1);
        return {RespObject::make_simple_string(line), crlf + 2};
    } else if (prefix == '-') {
        std::string line = buffer.substr(1, crlf - 1);
        return {RespObject::make_error(line), crlf + 2};
    } else if (prefix == ':') {
        std::string line = buffer.substr(1, crlf - 1);
        int64_t val = std::stoll(line);
        return {RespObject::make_integer(val), crlf + 2};
    } else if (prefix == '$') {
        std::string len_str = buffer.substr(1, crlf - 1);
        int len = std::stoi(len_str);
        if (len == -1) {
            return {RespObject::make_null(), crlf + 2};
        }
        size_t total_expected = crlf + 2 + static_cast<size_t>(len) + 2;
        if (buffer.length() < total_expected) {
            return {RespObject::make_null(), 0}; // Need more data
        }
        std::string content = buffer.substr(crlf + 2, static_cast<size_t>(len));
        return {RespObject::make_bulk_string(content), total_expected};
    } else if (prefix == '*') {
        std::string count_str = buffer.substr(1, crlf - 1);
        int count = std::stoi(count_str);
        if (count == -1) {
            return {RespObject::make_null(), crlf + 2};
        }
        
        size_t offset = crlf + 2;
        std::vector<RespObject> elements;
        elements.reserve(static_cast<size_t>(count));

        for (int i = 0; i < count; ++i) {
            auto [elem, bytes] = parse(buffer.substr(offset));
            if (bytes == 0) {
                return {RespObject::make_null(), 0}; // Incomplete sub-element
            }
            elements.push_back(elem);
            offset += bytes;
        }
        return {RespObject::make_array(elements), offset};
    }

    // Inline fallback (plain text input like "SET k v\r\n")
    std::string line = buffer.substr(0, crlf);
    std::stringstream ss(line);
    std::string token;
    std::vector<RespObject> tokens;
    while (ss >> token) {
        tokens.push_back(RespObject::make_bulk_string(token));
    }
    return {RespObject::make_array(tokens), crlf + 2};
}

} // namespace miniredis::protocol
