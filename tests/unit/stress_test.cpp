#include <gtest/gtest.h>
#include "storage/datastore.hpp"
#include "concurrency/thread_pool.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <random>

using namespace miniredis::storage;
using namespace miniredis::concurrency;

TEST(StressTest, HighConcurrencyRandomOperations) {
    DataStore store;
    ThreadPool pool(8); // 8 worker threads

    constexpr int total_ops = 50000;
    std::atomic<int> completed_ops{0};
    std::atomic<int> set_count{0};
    std::atomic<int> get_count{0};
    std::atomic<int> del_count{0};

    std::vector<std::future<void>> futures;
    futures.reserve(total_ops);

    for (int i = 0; i < total_ops; ++i) {
        futures.push_back(pool.enqueue([&store, &completed_ops, &set_count, &get_count, &del_count, i]() {
            int op_type = i % 3;
            std::string key = "stress_key_" + std::to_string(i % 100);

            if (op_type == 0) {
                store.set(key, "val_" + std::to_string(i));
                set_count++;
            } else if (op_type == 1) {
                auto res = store.get(key);
                get_count++;
            } else {
                store.del(key);
                del_count++;
            }
            completed_ops++;
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    EXPECT_EQ(completed_ops.load(), total_ops);
    EXPECT_GT(set_count.load(), 0);
    EXPECT_GT(get_count.load(), 0);
    EXPECT_GT(del_count.load(), 0);
}
