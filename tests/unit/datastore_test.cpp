#include <gtest/gtest.h>
#include "storage/datastore.hpp"
#include "commands/command.hpp"
#include <thread>
#include <vector>

using namespace miniredis::storage;
using namespace miniredis::commands;

TEST(DataStoreTest, BasicSetGetDel) {
    DataStore store;
    store.set("name", "Satyam");
    
    auto val = store.get("name");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, "Satyam");

    EXPECT_TRUE(store.exists("name"));
    EXPECT_EQ(store.dbsize(), 1);

    EXPECT_TRUE(store.del("name"));
    EXPECT_FALSE(store.exists("name"));
    EXPECT_EQ(store.dbsize(), 0);
}

TEST(DataStoreTest, TtlAndLazyExpiration) {
    DataStore store;
    store.set("session", "abc123", 1); // 1 second TTL

    EXPECT_TRUE(store.exists("session"));
    EXPECT_GT(store.ttl("session"), 0);

    // Sleep for 1.1s to allow expiration
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    EXPECT_FALSE(store.get("session").has_value());
    EXPECT_FALSE(store.exists("session"));
    EXPECT_EQ(store.ttl("session"), -2);
}

TEST(DataStoreTest, ConcurrentReadsAndWrites) {
    DataStore store;
    constexpr int num_threads = 10;
    constexpr int ops_per_thread = 1000;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&store, i]() {
            for (int j = 0; j < ops_per_thread; ++j) {
                std::string key = "key_" + std::to_string(i) + "_" + std::to_string(j);
                store.set(key, "val");
                auto v = store.get(key);
                EXPECT_TRUE(v.has_value());
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(store.dbsize(), static_cast<size_t>(num_threads * ops_per_thread));
}

TEST(CommandParserTest, ExecuteSetGet) {
    DataStore store;
    
    auto set_cmd = CommandRegistry::parse({"SET", "framework", "C++20"});
    std::string res1 = set_cmd->execute(store);
    EXPECT_EQ(res1, "+OK\r\n");

    auto get_cmd = CommandRegistry::parse({"GET", "framework"});
    std::string res2 = get_cmd->execute(store);
    EXPECT_EQ(res2, "$5\r\nC++20\r\n");
}
