#pragma once

#include <algorithm>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

#include "milmon/math/primality.hpp"
#include "milmon/misc/splitmix64.hpp"

namespace cp {
namespace detail {

inline u64 pollard_random() {
    static thread_local u64 state = 0x243f6a8885a308d3ULL;
    const u64 res = splitmix64(state);
    state += 0x9e3779b97f4a7c15ULL;
    return res;
}

inline u64 rho_step(u64 x, u64 c, u64 mod) { return u64((u128(x) * x + c) % mod); }

inline u64 abs_diff(u64 a, u64 b) { return a > b ? a - b : b - a; }

inline u64 pollard_rho_composite(u64 n) {
    constexpr u64 batch = 128;
    for (;;) {
        u64 y = pollard_random() % (n - 1) + 1;
        const u64 c = pollard_random() % (n - 1) + 1;
        u64 x = 0, saved = 0, factor = 1, cycle = 1;
        while (factor == 1) {
            x = y;
            for (u64 i = 0; i < cycle; ++i) y = rho_step(y, c, n);
            for (u64 off = 0; off < cycle && factor == 1; off += batch) {
                saved = y;
                u64 product = 1;
                const u64 count = std::min(batch, cycle - off);
                for (u64 i = 0; i < count; ++i) {
                    y = rho_step(y, c, n);
                    product = mul_mod(product, abs_diff(x, y), n);
                }
                factor = std::gcd(product, n);
            }
            if (cycle > std::numeric_limits<u64>::max() / 2) { factor = n; break; }
            cycle <<= 1U;
        }
        if (factor == n) {
            do {
                saved = rho_step(saved, c, n);
                factor = std::gcd(abs_diff(x, saved), n);
            } while (factor == 1);
        }
        if (factor != n) return factor;
    }
}

}

inline u64 pollard_rho(u64 n) {
    if (n < 2) return n;
    for (u64 p : detail::small_primes) if (n % p == 0) return p;
    if (detail::is_prime_u64(n)) return n;
    return detail::pollard_rho_composite(n);
}

namespace detail {

inline void factorize_into(u64 n, std::vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime_u64(n)) { factors.push_back(n); return; }
    const u64 factor = pollard_rho_composite(n);
    factorize_into(factor, factors);
    factorize_into(n / factor, factors);
}

}

inline std::vector<u64> factorize(u64 n) {
    std::vector<u64> res;
    if (n < 2) return res;
    res.reserve(16);
    for (u64 p : detail::small_primes) {
        while (n % p == 0) { res.push_back(p); n /= p; }
        if (n == 1) break;
    }
    if (n != 1) detail::factorize_into(n, res);
    std::sort(res.begin(), res.end());
    return res;
}

inline std::vector<std::pair<u64, int>> factorize_pair(u64 n) {
    std::vector<std::pair<u64, int>> res;
    for (u64 p : factorize(n)) {
        if (res.empty() || res.back().first != p) res.emplace_back(p, 1);
        else ++res.back().second;
    }
    return res;
}

}
