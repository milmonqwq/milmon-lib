#pragma once

#include <utility>

#include "milmon/types.hpp"

namespace cp {

inline i64 floor_sum(i64 n, i64 m, i64 a, i64 b) {
    i128 nn = n, mm = m, aa = a, bb = b, res = 0;
    i128 q = aa / mm;
    aa %= mm;
    if (aa < 0) { aa += mm; --q; }
    res += nn * (nn - 1) / 2 * q;
    q = bb / mm;
    bb %= mm;
    if (bb < 0) { bb += mm; --q; }
    res += nn * q;
    for (;;) {
        if (aa >= mm) { res += nn * (nn - 1) / 2 * (aa / mm); aa %= mm; }
        if (bb >= mm) { res += nn * (bb / mm); bb %= mm; }
        const i128 y = aa * nn + bb;
        if (y < mm) return i64(res);
        nn = y / mm;
        bb = y % mm;
        std::swap(aa, mm);
    }
}

}
