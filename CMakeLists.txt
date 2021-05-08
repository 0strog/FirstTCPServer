cmake_minimum_required(VERSION 3.5)

project(MyTCPServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FirstTCPServer
    main.cpp
    tcpserver.cpp
    tcpserver.h
    )
