project(fmt_formatter)
cmake_minimum_required(VERSION 3.22.1)

set(FMT_INSTALL OFF)
add_subdirectory(fmt-11.2.0)

add_library(fmt_formatter STATIC fmt_formatter.h fmt_formatter.cpp)
target_include_directories(fmt_formatter PUBLIC fmt-11.2.0/include)
target_link_libraries(fmt_formatter fmt::fmt)

if(DEFINED SHOW_ADVANCED_LOGS AND SHOW_ADVANCED_LOGS)
    target_compile_definitions(fmt_formatter PUBLIC LOG_FORMATTING_ERR=1)
else ()
    target_compile_definitions(fmt_formatter PUBLIC LOG_FORMATTING_ERR=0)
endif()

#-fPIC
if (UNIX)
    set_target_properties(fmt_formatter PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()