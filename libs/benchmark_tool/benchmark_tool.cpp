//
// Created by Kaspek on 25.09.2022.
//

#include "benchmark_tool.h"

BenchmarkTool::BenchmarkTool(fmt::color color, std::string scope) {     // Constructor
    start = std::chrono::high_resolution_clock::now();
    startTotal = start;
    this->color = color;
    this->scope = std::move(scope);
}

void BenchmarkTool::reset() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - startTotal).count();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTotal).count();

    std::string breakpoints_;
    if (!breakpoints.empty()) {
        breakpoints_ = " [";
        for (const auto &item: this->breakpoints) {
            breakpoints_ += std::to_string(item.first) + ": " + std::to_string(item.second) + "μs, ";
        }
        // remove last ", "
        breakpoints_.pop_back();
        breakpoints_.pop_back();
        breakpoints_ += "]";
    }
    dbg_println("{} took total {} microseconds ({} ms) {}", scope, duration, duration_ms, breakpoints_);

    start = std::chrono::high_resolution_clock::now();
    startTotal = start;
    count = 0;
}

void BenchmarkTool::breakpoint(int id) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    breakpoints.insert(std::pair<int, long>(id, duration));

    start = std::chrono::high_resolution_clock::now();
}

BenchmarkTool::BenchmarkTool() {
    start = std::chrono::high_resolution_clock::now();
    startTotal = start;

    color = fmt::color::white;
}

long BenchmarkTool::get_duration() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTotal).count();

    start = std::chrono::high_resolution_clock::now();
    startTotal = start;

    return duration;
}
