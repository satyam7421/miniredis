cmake_minimum_required(VERSION 3.20)
project(miniredis VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

# Directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core Library Sources
file(GLOB_RECURSE SOURCES 
    "src/storage/*.cpp"
    "src/commands/*.cpp"
    "src/protocol/*.cpp"
    "src/network/*.cpp"
    "src/concurrency/*.cpp"
    "src/persistence/*.cpp"
    "src/utils/*.cpp"
)

add_library(miniredis_core ${SOURCES})

if(WIN32)
    target_link_libraries(miniredis_core PUBLIC ws2_32)
endif()

# Server Executable
add_executable(miniredis_server src/main.cpp)
target_link_libraries(miniredis_server PRIVATE miniredis_core)

# CLI Client Executable
add_executable(miniredis_cli src/cli_main.cpp)
target_link_libraries(miniredis_cli PRIVATE miniredis_core)

# Benchmark Client Executable
add_executable(miniredis_benchmark src/benchmark_main.cpp)
target_link_libraries(miniredis_benchmark PRIVATE miniredis_core)

# Enable Testing
enable_testing()
add_subdirectory(tests)


