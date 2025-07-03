cmake_minimum_required(VERSION 3.22)
project(kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kv_server src/server.cpp)
add_executable(kv_client src/client.cpp)