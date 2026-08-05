#pragma once

#include <cassert>
#include <type_traits>

namespace cp {
namespace detail {

template <typename T> inline T frac_gcd(T a, T b) {
    while (b != 0) { const T r = a % b; a = b; b = r; }
    return a < 0 ? -a : a;
}

}

template <typename T> struct frac {
    static_assert((std::is_integral_v<T> && std::is_signed_v<T>) || std::is_same_v<T, __int128_t>,
                  "frac requires a signed integer type");
    using value_type = T;
    T num, den;

    frac(T n = 0, T d = 1) : num(n), den(d) {
        assert(den != 0);
        const T g = detail::frac_gcd(num, den);
        num /= g; den /= g;
        if (den < 0) { num = -num; den = -den; }
    }

    inline frac operator+() const { return *this; }

    inline frac operator-() const { return normalized(-num, den); }

    inline frac& operator+=(frac b) { return *this = *this + b; }

    inline frac& operator-=(frac b) { return *this = *this - b; }

    inline frac& operator*=(frac b) { return *this = *this * b; }

    inline frac& operator/=(frac b) { return *this = *this / b; }

    inline long double value() const { return static_cast<long double>(num) / den; }

    friend inline frac operator+(frac a, frac b) {
        const T g = detail::frac_gcd(a.den, b.den);
        const T ad = a.den / g, bd = b.den / g;
        const T n = a.num * bd + b.num * ad;
        const T h = detail::frac_gcd(n, g);
        return normalized(n / h, ad * (b.den / h));
    }

    friend inline frac operator-(frac a, frac b) { return a + -b; }

    friend inline frac operator*(frac a, frac b) {
        const T g1 = detail::frac_gcd(a.num, b.den), g2 = detail::frac_gcd(b.num, a.den);
        return normalized(a.num / g1 * (b.num / g2), a.den / g2 * (b.den / g1));
    }

    friend inline frac operator/(frac a, frac b) {
        assert(b.num != 0);
        const T g1 = detail::frac_gcd(a.num, b.num), g2 = detail::frac_gcd(a.den, b.den);
        return normalized(a.num / g1 * (b.den / g2), a.den / g2 * (b.num / g1));
    }

    friend inline bool operator==(frac a, frac b) { return a.num == b.num && a.den == b.den; }

    friend inline bool operator!=(frac a, frac b) { return !(a == b); }

    friend inline bool operator<(frac a, frac b) {
        using W = std::conditional_t<(sizeof(T) < sizeof(long long)), long long, __int128_t>;
        return W(a.num) * b.den < W(b.num) * a.den;
    }

    friend inline bool operator>(frac a, frac b) { return b < a; }

    friend inline bool operator<=(frac a, frac b) { return !(b < a); }

    friend inline bool operator>=(frac a, frac b) { return !(a < b); }

private:
    struct normalized_tag {};

    frac(T n, T d, normalized_tag) : num(n), den(d) {
        assert(den != 0);
        if (den < 0) { num = -num; den = -den; }
    }

    static inline frac normalized(T n, T d) { return frac(n, d, normalized_tag{}); }
};

}
