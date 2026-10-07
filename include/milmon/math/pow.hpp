#pragma once

#include "milmon/types.hpp"

namespace cp {

template <u32 P>
inline u32 pow(u32 x, u32 y) {
    static_assert(P > 1, "pow requires a modulus greater than 1");
    u32 res = 1;
    x %= P;
    while (y != 0) {
        if (y & 1U) res = u64(res) * x % P;
        x = u64(x) * x % P;
        y >>= 1U;
    }
    return res;
}

template <u32 P>
inline u32 inv(u32 x) {
    static_assert(P > 1, "inv requires a modulus greater than 1");
    return pow<P>(x, P - 2);
}

}
