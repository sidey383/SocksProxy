cmake_minimum_required(VERSION 3.20)
project(SocksProxy)

set(CMAKE_CXX_STANDARD 17)

add_executable(SocksProxy
        src/main.cpp
        src/SocksServer.cpp
        tcp/Socket.cpp
        tcp/Server.cpp
        src/SocksWorker.cpp)
