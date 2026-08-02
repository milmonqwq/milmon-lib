#pragma once

#include <type_traits>

namespace cp {

template <class T>
inline T exgcd(T a, T b, T& x, T& y) {
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>, "exgcd requires a signed integer type");
    using W = std::conditional_t<(sizeof(T) < sizeof(long long)), long long, __int128_t>;
    W r0 = a, r1 = b, x0 = 1, x1 = 0, y0 = 0, y1 = 1;
    while (r1 != 0) {
        const W q = r0 / r1;
        const W r2 = r0 - q * r1, x2 = x0 - q * x1, y2 = y0 - q * y1;
        r0 = r1; r1 = r2;
        x0 = x1; x1 = x2;
        y0 = y1; y1 = y2;
    }
    if (r0 < 0) { r0 = -r0; x0 = -x0; y0 = -y0; }
    x = T(x0); y = T(y0);
    return T(r0);
}

}
