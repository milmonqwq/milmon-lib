#pragma once

namespace cp {

struct p2 {
    int x, y;
};

inline bool operator==(p2 a, p2 b) { return a.x == b.x && a.y == b.y; }

inline bool operator!=(p2 a, p2 b) { return !(a == b); }

inline bool operator<(p2 a, p2 b) { return a.x != b.x ? a.x < b.x : a.y < b.y; }

inline p2 operator+(p2 a, p2 b) { return {a.x + b.x, a.y + b.y}; }

inline p2 operator-(p2 a, p2 b) { return {a.x - b.x, a.y - b.y}; }

inline p2 operator-(p2 a) { return {-a.x, -a.y}; }

inline p2& operator+=(p2& a, p2 b) { return a = a + b; }

inline p2& operator-=(p2& a, p2 b) { return a = a - b; }

inline p2 operator*(p2 a, int k) { return {a.x * k, a.y * k}; }

inline p2 operator*(int k, p2 a) { return a * k; }

inline p2& operator*=(p2& a, int k) { return a = a * k; }

inline long long dot(p2 a, p2 b) { return 1LL * a.x * b.x + 1LL * a.y * b.y; }

inline long long cross(p2 a, p2 b) { return 1LL * a.x * b.y - 1LL * a.y * b.x; }

inline long long cross(p2 o, p2 a, p2 b) {
    const long long ax = 1LL * a.x - o.x, ay = 1LL * a.y - o.y;
    const long long bx = 1LL * b.x - o.x, by = 1LL * b.y - o.y;
    return ax * by - ay * bx;
}

}
