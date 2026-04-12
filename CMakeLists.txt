cmake_minimum_required(VERSION 3.16)

project(MatchingEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server
add_executable(matching_engine_server
    apps/matching_engine_server/main.cpp
    src/domain/Order.cpp
)

target_include_directories(matching_engine_server PRIVATE
    include
)
