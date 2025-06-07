cmake_minimum_required(VERSION 3.24)
project(MiniSPA)

include_directories(libs/fmt_formatter)
include_directories(libs/benchmark_tool)

add_subdirectory(libs/fmt_formatter)
add_subdirectory(libs/benchmark_tool)



set(CMAKE_CXX_STANDARD 17)

add_executable(MiniSPA parser.cpp parser.h nodes.h main.cpp utils.cpp utils.h
        nodes.cpp pkb.cpp pkb.h Query/query.cpp Query/query.h
        Query/Instruction.cpp
        Query/Instruction.h
        Query/SubInstruction.cpp
        Query/SubInstruction.h
        Query/SynonymConstraint.cpp
        Query/SynonymConstraint.h)

target_link_libraries(MiniSPA fmt_formatter benchmark_tool -static)
