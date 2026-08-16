#include <gtest/gtest.h>
#include "protocol/resp.hpp"
#include "concurrency/thread_pool.hpp"
#include "persistence/aof.hpp"
#include <atomic>
#include <filesystem>

using namespace miniredis::protocol;
using namespace miniredis::concurrency;
using namespace miniredis::persistence;

TEST(RespParserTest, SimpleStringParsing) {
    auto [obj, bytes] = RespParser::parse("+OK\r\n");
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(obj.type, RespType::SimpleString);
    EXPECT_EQ(obj.value, "OK");
}

TEST(RespParserTest, BulkStringParsing) {
    auto [obj, bytes] = RespParser::parse("$6\r\nSatyam\r\n");
    EXPECT_EQ(bytes, 12);
    EXPECT_EQ(obj.type, RespType::BulkString);
    EXPECT_EQ(obj.value, "Satyam");
}

TEST(RespParserTest, ArrayParsing) {
    std::string resp = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$6\r\nSatyam\r\n";
    auto [obj, bytes] = RespParser::parse(resp);
    EXPECT_EQ(bytes, resp.length());
    EXPECT_EQ(obj.type, RespType::Array);
    ASSERT_EQ(obj.array_value.size(), 3);
    EXPECT_EQ(obj.array_value[0].value, "SET");
    EXPECT_EQ(obj.array_value[1].value, "name");
    EXPECT_EQ(obj.array_value[2].value, "Satyam");
}

TEST(ThreadPoolTest, ExecuteTasksConcurrently) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    constexpr int total_tasks = 100;

    std::vector<std::future<void>> futures;
    futures.reserve(total_tasks);

    for (int i = 0; i < total_tasks; ++i) {
        futures.push_back(pool.enqueue([&counter]() {
            counter++;
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    EXPECT_EQ(counter.load(), total_tasks);
}

TEST(AofTest, AppendAndVerify) {
    std::string test_file = "test_append.aof";
    {
        AofManager aof(test_file);
        aof.append("*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$1\r\nv\r\n");
    }

    EXPECT_TRUE(std::filesystem::exists(test_file));
    std::filesystem::remove(test_file);
}
