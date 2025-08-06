cmake_minimum_required(VERSION 3.22)
project(kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(kv_tests tests/kv_tests.cpp src/kv_store.cpp src/ThreadPool.cpp)
target_link_libraries(kv_tests gtest_main)

add_executable(kv_server src/kv_store.cpp src/ThreadPool.cpp)
add_executable(kv_client src/client.cpp)