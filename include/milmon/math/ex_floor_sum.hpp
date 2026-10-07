#pragma once

#include <tuple>

#include "milmon/types.hpp"

namespace cp {

inline std::tuple<i128, i128, i128> ex_floor_sum(i64 n, i64 m, i64 a, i64 b) {
    if (n == 0) return {0, 0, 0};
    i64 qa = a / m;
    a %= m;
    if (a < 0) { a += m; --qa; }
    i64 qb = b / m;
    b %= m;
    if (b < 0) { b += m; --qb; }
    i128 s0 = 0, s1 = 0, s2 = 0;
    const i128 y = i128(a) * n + b;
    if (y >= m) {
        const i64 nn = i64(y / m);
        auto [t0, t1, t2] = ex_floor_sum(nn, a, m, i64(y % m));
        s0 = t0;
        s1 = (i128(2) * n * t0 - t0 - t2) / 2;
        s2 = (i128(2) * nn - 1) * t0 - 2 * t1;
    }
    const i128 si = i128(n) * (n - 1) / 2;
    if (qa != 0) {
        const i128 si2 = i128(n) * (n - 1) * (i128(2) * n - 1) / 6;
        s2 += i128(qa) * qa * si2 + i128(2) * qa * s1;
        s1 += i128(qa) * si2;
        s0 += i128(qa) * si;
    }
    if (qb != 0) {
        s2 += i128(qb) * qb * n + i128(2) * qb * s0;
        s1 += i128(qb) * si;
        s0 += i128(qb) * n;
    }
    return {s0, s1, s2};
}

}
