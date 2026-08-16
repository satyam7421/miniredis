#include <gtest/gtest.h>
#include "storage/datastore.hpp"
#include "persistence/aof.hpp"
#include <filesystem>

using namespace miniredis::storage;
using namespace miniredis::persistence;

TEST(PersistenceTest, AofSaveAndReplay) {
    std::string test_aof = "test_replay.aof";
    if (std::filesystem::exists(test_aof)) {
        std::filesystem::remove(test_aof);
    }

    {
        AofManager aof(test_aof);
        aof.append("*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$6\r\nSatyam\r\n");
        aof.append("*3\r\n$3\r\nSET\r\n$3\r\nage\r\n$2\r\n22\r\n");
        aof.close();
    }

    DataStore restored_store;
    AofManager replay_aof(test_aof);
    size_t replayed = replay_aof.load_and_replay(restored_store);
    replay_aof.close();

    EXPECT_EQ(replayed, 2);
    EXPECT_EQ(restored_store.get("name").value_or(""), "Satyam");
    EXPECT_EQ(restored_store.get("age").value_or(""), "22");

    std::filesystem::remove(test_aof);

}

TEST(PersistenceTest, SnapshotSaveAndLoad) {
    std::string test_rdb = "test_dump.rdb";
    if (std::filesystem::exists(test_rdb)) {
        std::filesystem::remove(test_rdb);
    }

    DataStore store;
    store.set("k1", "v1");
    store.set("k2", "v2");

    ASSERT_TRUE(SnapshotManager::save(store, test_rdb));

    DataStore loaded_store;
    ASSERT_TRUE(SnapshotManager::load(loaded_store, test_rdb));

    EXPECT_EQ(loaded_store.dbsize(), 2);
    EXPECT_EQ(loaded_store.get("k1").value_or(""), "v1");
    EXPECT_EQ(loaded_store.get("k2").value_or(""), "v2");

    std::filesystem::remove(test_rdb);
}
