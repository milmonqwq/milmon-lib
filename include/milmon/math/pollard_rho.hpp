#pragma once

#include <algorithm>
#include <limits>
#include <numeric>
#include <vector>

#include "milmon/math/primality.hpp"

namespace cp {
namespace detail {

inline u64 splitmix64(u64& state) {
    u64 x = (state += 0x9e3779b97f4a7c15ULL);
    x = (x ^ (x >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27U)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31U);
}

inline u64 pollard_random() { static thread_local u64 state = 0x243f6a8885a308d3ULL; return splitmix64(state); }

inline u64 rho_step(u64 x, u64 c, u64 mod) { return u64((u128(x) * x + c) % mod); }

inline u64 abs_diff(u64 a, u64 b) { return a > b ? a - b : b - a; }

}

inline u64 pollard_rho(u64 n) {
    if (n < 2) return n;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL,
                  19ULL, 23ULL, 29ULL, 31ULL, 37ULL})
        if (n % p == 0) return p;
    if (detail::is_prime_u64(n)) return n;
    constexpr u64 batch = 128;
    for (;;) {
        u64 y = detail::pollard_random() % (n - 1) + 1;
        const u64 c = detail::pollard_random() % (n - 1) + 1;
        u64 x = 0, saved = 0, factor = 1, cycle = 1;
        while (factor == 1) {
            x = y;
            for (u64 i = 0; i < cycle; ++i) y = detail::rho_step(y, c, n);
            for (u64 off = 0; off < cycle && factor == 1; off += batch) {
                saved = y;
                u64 product = 1;
                const u64 count = std::min(batch, cycle - off);
                for (u64 i = 0; i < count; ++i) {
                    y = detail::rho_step(y, c, n);
                    product = detail::mul_mod(product, detail::abs_diff(x, y), n);
                }
                factor = std::gcd(product, n);
            }
            if (cycle > std::numeric_limits<u64>::max() / 2) { factor = n; break; }
            cycle <<= 1U;
        }
        if (factor == n) {
            do {
                saved = detail::rho_step(saved, c, n);
                factor = std::gcd(detail::abs_diff(x, saved), n);
            } while (factor == 1);
        }
        if (factor != n) return factor;
    }
}

namespace detail {

inline void factorize_into(u64 n, std::vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime_u64(n)) { factors.push_back(n); return; }
    const u64 factor = pollard_rho(n);
    factorize_into(factor, factors);
    factorize_into(n / factor, factors);
}

}

inline std::vector<u64> factorize(u64 n) {
    std::vector<u64> res;
    if (n >= 2) { detail::factorize_into(n, res); std::sort(res.begin(), res.end()); }
    return res;
}

}
