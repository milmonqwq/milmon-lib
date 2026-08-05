#pragma once

#include <utility>

#include "milmon/types.hpp"

namespace cp {

inline i64 floor_sum(i64 n, i64 m, i64 a, i64 b) {
    i128 res = 0;
    i64 q = a / m;
    a %= m;
    if (a < 0) { a += m; --q; }
    res += i128(n) * (i128(n) - 1) / 2 * q;
    q = b / m;
    b %= m;
    if (b < 0) { b += m; --q; }
    res += i128(n) * q;
    for (;;) {
        const i128 y = i128(a) * n + b;
        if (y < m) return i64(res);
        n = i64(y / m);
        b = i64(y % m);
        std::swap(a, m);
        res += i128(n) * (n - 1) / 2 * (a / m);
        a %= m;
        res += i128(n) * (b / m);
        b %= m;
    }
}

}
