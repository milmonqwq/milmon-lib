#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "milmon/types.hpp"

namespace cp {
namespace poly {
namespace detail {

inline constexpr u32 mod = 998244353;
inline constexpr u32 primitive_root = 3;
inline constexpr int max_power = 23;
inline std::array<u32, max_power + 1> roots{};
inline std::array<u32, max_power + 1> inv_roots{};

inline u32 pow_mod(u32 a, u32 e) {
    u32 res = 1;
    while (e != 0) {
        if (e & 1U) res = u64(res) * a % mod;
        a = u64(a) * a % mod;
        e >>= 1U;
    }
    return res;
}

inline u32 normalize(u32 x) { return x % mod; }

inline void ntt(std::vector<u32>& a, bool invert) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1U;
        while (j & bit) { j ^= bit; bit >>= 1U; }
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    int k = 1;
    for (std::size_t len = 2; len <= n; len <<= 1U, ++k) {
        const std::size_t half = len >> 1U;
        const u32 step = invert ? inv_roots[k] : roots[k];
        for (std::size_t i = 0; i < n; i += len) {
            u32 w = 1;
            for (std::size_t j = 0; j < half; ++j) {
                const u32 x = a[i + j], y = u64(a[i + j + half]) * w % mod;
                u32 sum = x + y, diff = x + mod - y;
                if (sum >= mod) sum -= mod;
                if (diff >= mod) diff -= mod;
                a[i + j] = sum;
                a[i + j + half] = diff;
                w = u64(w) * step % mod;
            }
        }
    }
    if (invert) {
        const u32 inv_n = pow_mod(n, mod - 2);
        for (u32& x : a) x = u64(x) * inv_n % mod;
    }
}

}

inline void init() {
    static const bool initialized = [] {
        for (int k = 1; k <= detail::max_power; ++k) {
            detail::roots[k] = detail::pow_mod(detail::primitive_root, (detail::mod - 1) >> k);
            detail::inv_roots[k] = detail::pow_mod(detail::roots[k], detail::mod - 2);
        }
        return true;
    }();
    (void)initialized;
}

inline std::vector<u32> poly_mul(const std::vector<u32>& a, const std::vector<u32>& b) {
    if (a.empty() || b.empty()) return {};
    init();
    const std::size_t size = a.size() + b.size() - 1;
    std::size_t n = 1;
    while (n < size) n <<= 1U;
    if (n > (std::size_t(1) << detail::max_power)) throw std::length_error("polynomial product exceeds the NTT limit");
    std::vector<u32> x(n), y(n);
    for (std::size_t i = 0; i < a.size(); ++i) x[i] = detail::normalize(a[i]);
    for (std::size_t i = 0; i < b.size(); ++i) y[i] = detail::normalize(b[i]);
    detail::ntt(x, false);
    detail::ntt(y, false);
    for (std::size_t i = 0; i < n; ++i) x[i] = u64(x[i]) * y[i] % detail::mod;
    detail::ntt(x, true);
    std::vector<u32> res(size);
    for (std::size_t i = 0; i < size; ++i) res[i] = x[i];
    return res;
}

}
}
