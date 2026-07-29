#pragma once

#include <initializer_list>
#include <type_traits>

#include "milmon/types.hpp"

namespace cp {
namespace detail {

inline constexpr u64 small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

inline u64 mul_mod(u64 a, u64 b, u64 mod) { return u64(u128(a) * b % mod); }

inline u64 pow_mod(u64 a, u64 e, u64 mod) {
    u64 res = 1;
    while (e != 0) {
        if (e & 1U) res = mul_mod(res, a, mod);
        a = mul_mod(a, a, mod);
        e >>= 1U;
    }
    return res;
}

inline bool is_prime_u64(u64 n) {
    if (n < 2) return false;
    for (u64 p : small_primes) {
        if (n % p == 0) return n == p;
    }
    const int s = __builtin_ctzll(n - 1);
    const u64 d = (n - 1) >> s;
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL,
                  450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = pow_mod(a % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool pass = false;
        for (int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) { pass = true; break; }
        }
        if (!pass) return false;
    }
    return true;
}

}

template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
inline bool is_prime(T n) {
    static_assert(sizeof(T) <= sizeof(u64), "is_prime supports integers of at most 64 bits");
    if constexpr (std::is_signed_v<T>) if (n < 0) return false;
    return detail::is_prime_u64(u64(n));
}

}
