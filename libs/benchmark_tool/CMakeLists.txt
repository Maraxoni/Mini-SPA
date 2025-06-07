add_library(benchmark_tool STATIC benchmark_tool.cpp benchmark_tool.h)

target_link_libraries(benchmark_tool fmt_formatter)

#-fPIC
if (UNIX)
    set_target_properties(benchmark_tool PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(DEFINED SHOW_ADVANCED_LOGS AND SHOW_ADVANCED_LOGS)
    target_compile_definitions(benchmark_tool PRIVATE SHOW_BENCHMARKS=1)
else ()
    target_compile_definitions(benchmark_tool PRIVATE SHOW_BENCHMARKS=0)
endif()
