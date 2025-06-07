//
// Created by Kaspek on 04.02.2023.
//

#ifndef SHIELDYREWRITTENMODULE_FMT_FORMATTER_H
#define SHIELDYREWRITTENMODULE_FMT_FORMATTER_H

#include "fmt/core.h"
#include "fmt/color.h"
#include "fmt/chrono.h"
#include <iomanip>
//#include <sstream>
//
//#if defined(_WIN32) || defined(_WIN64)
//
//#include <windows.h>
//
//#endif

extern bool terminal_color_supported;
extern bool windows_terminal_color_fixed;

void fix_windows_colors();

template<typename... Args>
void fmt_println(const fmt::text_style &ts, const Args &... args) {
    fix_windows_colors();
    try {
        fmt::print(terminal_color_supported ? ts : fmt::text_style(), args...);
        fmt::print("\n");
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::red), "Exception in fmt: {} for arg0: {}\n", e.what(), args...);
    }

    fflush(stdout);
}

template<typename... Args>
void fmt_println(const Args &... args) {
    fix_windows_colors();
    try {
        fmt::print(fmt::text_style(), args...);
        fmt::print("\n");
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::red), "Exception in fmt: {} for arg0: {}\n", e.what(), args...);
    }
    fflush(stdout);
}

template<typename... Args>
void dbg_println(const Args &... args) {

    fix_windows_colors();
    try {
        fmt::print(fmt::fg(fmt::color::light_green), args...);
        fmt::print("\n");
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::red), "Exception in dbg fmt: {} for arg0: {}\n", e.what(), args...);
    }
    fflush(stdout);
}

template<typename... Args>
void fmt_log(const fmt::text_style &ts, const Args &... args) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
    auto str = oss.str();

    fmt::print("[{}] ", str);
    fmt::print(ts, args...);
    fmt::print("\n");
    fflush(stdout);
}

bool is_utf8(const char *string);

/*template <typename T>
struct myFormatter : fmt::formatter<std::vector<T>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return fmt::formatter<std::string>::parse(ctx);
    }

    // Formatujemy wektor.
    template <typename FormatContext>
    auto format(const std::vector<std::string>& vec, FormatContext& ctx) {
        // Rozpoczynamy sekcję listy.
        auto out = ctx.out();
        *out++ = '[';

        // Formatujemy elementy wektora.
        for (const auto& element : vec) {
            out = fmt::format_to(out, " \"{}\"", element);
        }

        // Zamykamy sekcję listy.
        *out++ = ']';
        return out;
    }
};*/

#endif //SHIELDYREWRITTENMODULE_FMT_FORMATTER_H