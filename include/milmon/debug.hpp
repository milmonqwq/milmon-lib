#pragma once

#include <cstdio>
#include <iostream>
#include <type_traits>
#include <utility>

namespace cp {
namespace detail {

template<typename T, typename = void> struct is_debug_container : std::false_type {};

template<typename T> struct is_debug_container<T, std::void_t<decltype(std::declval<const T&>().begin()), decltype(std::declval<const T&>().end())>> : std::true_type {};

template<typename T, typename = void> struct has_substr : std::false_type {};

template<typename T> struct has_substr<T, std::void_t<decltype(std::declval<const T&>().substr())>> : std::true_type {};

}
}

template<typename T, std::enable_if_t<cp::detail::is_debug_container<T>::value && !cp::detail::has_substr<T>::value, int> = 0>
inline std::ostream& operator<<(std::ostream& os, const T& v) {
    os << '[';
    int f = 0;
    for (const auto& x : v) os << (f++ ? "," : "") << x;
    return os << ']';
}

template<typename... Args> inline void debug_impl(const char* s, const Args&... xs) {
    auto f = [&](const auto& x) {
        while (*s == ' ' || *s == '\t') ++s;
        const char* l = s;
        int dep = 0;
        char quote = 0;
        bool escaped = false;
        while (*s && !(*s == ',' && dep == 0 && quote == 0)) {
            const char c = *s++;
            if (quote) {
                if (escaped) escaped = false;
                else if (c == '\\') escaped = true;
                else if (c == quote) quote = 0;
            } else if (c == '\'' || c == '"') quote = c;
            else if (c == '(' || c == '[' || c == '{') ++dep;
            else if (c == ')' || c == ']' || c == '}') --dep;
        }
        const char* r = s;
        while (r > l && (r[-1] == ' ' || r[-1] == '\t')) --r;
        std::cerr.write(l, r - l) << '=' << x;
        if (*s == ',') { ++s; std::cerr << ", "; }
        else std::cerr << '\n';
    };
    (f(xs), ...);
}

#define debug(...) std::fprintf(stderr, __VA_ARGS__)
#define dbg(...) debug_impl(#__VA_ARGS__, __VA_ARGS__)
