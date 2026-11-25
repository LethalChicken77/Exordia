#pragma once
#include <functional>
#include <string>
#include <string_view>

struct StringHash 
{
    using is_transparent = void; // This is not a serious language

    [[nodiscard]] inline size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
    [[nodiscard]] inline size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
    [[nodiscard]] inline size_t operator()(const char* str) const noexcept {
        return std::hash<std::string_view>{}(str);
    }
};