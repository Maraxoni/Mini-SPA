cmake_minimum_required(VERSION 3.24)
project(MiniSPA)

set(CMAKE_CXX_STANDARD 17)

add_executable(MiniSPA parser.cpp parser.h nodes.h main.cpp utils.cpp utils.h
        nodes.cpp)
