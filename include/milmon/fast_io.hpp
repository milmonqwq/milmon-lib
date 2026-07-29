#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

namespace cp {

class FastScanner {
public:
    explicit FastScanner(std::FILE* f = stdin) : file(f) {}

    template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
    inline bool read(T& value) {
        int c = skip();
        if (c == EOF) return false;
        bool neg = false;
        if (c == '-' || c == '+') {
            neg = c == '-';
            c = next();
        }
        if (c < '0' || c > '9') return false;
        using U = std::make_unsigned_t<T>;
        U x = 0;
        do {
            x = x * 10 + c - '0';
            c = next();
        } while (c >= '0' && c <= '9');
        if (c > ' ') unread();
        if constexpr (std::is_signed_v<T>)
            value = neg ? T(U{0} - x) : T(x);
        else
            value = neg ? U{0} - x : x;
        return true;
    }

    inline bool read(std::string& s) {
        int c = skip();
        if (c == EOF) return false;
        s.clear();
        do {
            s.push_back(char(c));
            c = next();
        } while (c > ' ');
        return true;
    }

    inline bool read(char& value) {
        const int c = skip();
        if (c == EOF) return false;
        value = char(c);
        return true;
    }

    template <class T> inline FastScanner& operator>>(T& value) { read(value); return *this; }

private:
    static constexpr std::size_t buffer_size = 1U << 16U;

    inline int next() {
        if (pos == len) {
            len = std::fread(buf, 1, buffer_size, file);
            pos = 0;
            if (len == 0) return EOF;
        }
        return static_cast<unsigned char>(buf[pos++]);
    }

    inline void unread() { --pos; }

    inline int skip() {
        int c;
        do c = next(); while (c != EOF && c <= ' ');
        return c;
    }

    std::FILE* file;
    char buf[buffer_size]{};
    std::size_t pos = 0, len = 0;
};

class FastOutput {
public:
    explicit FastOutput(std::FILE* f = stdout) : file(f) {}

    ~FastOutput() { flush(); }

    FastOutput(const FastOutput&) = delete;
    FastOutput& operator=(const FastOutput&) = delete;

    inline void flush() { if (pos != 0) { std::fwrite(buf, 1, pos, file); pos = 0; } }

    inline void write(char c) { if (pos == buffer_size) flush(); buf[pos++] = c; }

    inline void write(std::string_view s) {
        if (s.empty()) return;
        if (s.size() >= buffer_size) {
            flush();
            std::fwrite(s.data(), 1, s.size(), file);
            return;
        }
        if (pos + s.size() > buffer_size) flush();
        std::memcpy(buf + pos, s.data(), s.size());
        pos += s.size();
    }

    template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
    inline void write(T value) {
        using U = std::make_unsigned_t<T>;
        U x;
        if constexpr (std::is_signed_v<T>) {
            if (value < 0) {
                write('-');
                x = U{0} - U(value);
            } else x = U(value);
        } else x = value;
        char digits[32];
        char* p = digits + sizeof(digits);
        do {
            *--p = char('0' + x % 10);
            x /= 10;
        } while (x != 0);
        write(std::string_view(p, digits + sizeof(digits) - p));
    }

    inline FastOutput& operator<<(char value) { write(value); return *this; }

    inline FastOutput& operator<<(const char* value) { write(value); return *this; }

    inline FastOutput& operator<<(std::string_view value) { write(value); return *this; }

    inline FastOutput& operator<<(const std::string& value) { write(value); return *this; }

    template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>, int> = 0>
    inline FastOutput& operator<<(T value) { write(value); return *this; }

private:
    static constexpr std::size_t buffer_size = 1U << 16U;

    std::FILE* file;
    char buf[buffer_size]{};
    std::size_t pos = 0;
};

}
