cmake_minimum_required(VERSION 3.8)
project(PJC-sem-wordcount-szelemar)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
set(CMAKE_CXX_STANDARD 14)


set(SOURCE_FILES main.cpp controller.cpp controller.hpp Command.cpp)
add_executable(PJC-sem-wordcount-szelemar ${SOURCE_FILES})