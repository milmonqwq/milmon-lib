#pragma once

#include <type_traits>

namespace cp {

template <typename T> struct p2r {
    static_assert(std::is_floating_point_v<T>, "p2r requires a floating-point type");
    using value_type = T;
    T x, y;
};

template <typename T> inline bool operator==(p2r<T> a, p2r<T> b) { return a.x == b.x && a.y == b.y; }

template <typename T> inline bool operator!=(p2r<T> a, p2r<T> b) { return !(a == b); }

template <typename T> inline bool operator<(p2r<T> a, p2r<T> b) { return a.x != b.x ? a.x < b.x : a.y < b.y; }

template <typename T> inline p2r<T> operator+(p2r<T> a, p2r<T> b) { return {a.x + b.x, a.y + b.y}; }

template <typename T> inline p2r<T> operator-(p2r<T> a, p2r<T> b) { return {a.x - b.x, a.y - b.y}; }

template <typename T> inline p2r<T> operator-(p2r<T> a) { return {-a.x, -a.y}; }

template <typename T> inline p2r<T>& operator+=(p2r<T>& a, p2r<T> b) { return a = a + b; }

template <typename T> inline p2r<T>& operator-=(p2r<T>& a, p2r<T> b) { return a = a - b; }

template <typename T> inline p2r<T> operator*(p2r<T> a, typename p2r<T>::value_type k) { return {a.x * k, a.y * k}; }

template <typename T> inline p2r<T> operator*(typename p2r<T>::value_type k, p2r<T> a) { return a * k; }

template <typename T> inline p2r<T>& operator*=(p2r<T>& a, typename p2r<T>::value_type k) { return a = a * k; }

template <typename T> inline p2r<T> operator/(p2r<T> a, typename p2r<T>::value_type k) { return {a.x / k, a.y / k}; }

template <typename T> inline p2r<T>& operator/=(p2r<T>& a, typename p2r<T>::value_type k) { return a = a / k; }

template <typename T> inline T dot(p2r<T> a, p2r<T> b) { return a.x * b.x + a.y * b.y; }

template <typename T> inline T cross(p2r<T> a, p2r<T> b) { return a.x * b.y - a.y * b.x; }

template <typename T> inline T cross(p2r<T> o, p2r<T> a, p2r<T> b) { return cross(a - o, b - o); }

}
