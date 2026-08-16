cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows MSVC runtime override
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(datastore_test unit/datastore_test.cpp)
target_link_libraries(datastore_test PRIVATE miniredis_core gtest_main)

add_executable(system_test unit/system_test.cpp)
target_link_libraries(system_test PRIVATE miniredis_core gtest_main)

add_executable(network_test unit/network_test.cpp)
target_link_libraries(network_test PRIVATE miniredis_core gtest_main)

add_executable(persistence_test unit/persistence_test.cpp)
target_link_libraries(persistence_test PRIVATE miniredis_core gtest_main)

add_executable(stress_test unit/stress_test.cpp)
target_link_libraries(stress_test PRIVATE miniredis_core gtest_main)

include(GoogleTest)
gtest_discover_tests(datastore_test)
gtest_discover_tests(system_test)
gtest_discover_tests(network_test)
gtest_discover_tests(persistence_test)
gtest_discover_tests(stress_test)




