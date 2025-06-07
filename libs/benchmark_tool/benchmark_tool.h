//
// Created by Kaspek on 25.09.2022.
//

#ifndef SHIELDYRESTAPI_BENCHMARK_TOOL_H
#define SHIELDYRESTAPI_BENCHMARK_TOOL_H

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "fmt_formatter.h"

class BenchmarkTool {
public:
    BenchmarkTool(); //Delete default constructor
    BenchmarkTool(fmt::color color, std::string scope); //Constructor
    fmt::color color;

    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTotal;
    int count{};
    std::string scope;
    std::map<int, long> breakpoints = {};

    void reset();
    void breakpoint(int id);
    long get_duration();
};


#endif //SHIELDYRESTAPI_BENCHMARK_TOOL_H
