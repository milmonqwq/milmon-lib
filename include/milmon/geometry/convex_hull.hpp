#pragma once

#include <algorithm>
#include <vector>

#include "milmon/geometry/p2.hpp"

namespace cp {

inline std::vector<p2> convex_hull(std::vector<p2> ps) {
    std::sort(ps.begin(), ps.end());
    ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
    if (ps.size() <= 1) return ps;
    std::vector<p2> res;
    res.reserve(ps.size() + 1);
    for (p2 p : ps) {
        while (res.size() >= 2 && cross(res[res.size() - 2], res.back(), p) <= 0) res.pop_back();
        res.push_back(p);
    }
    const std::size_t lower = res.size();
    for (int i = int(ps.size()) - 2; i >= 0; --i) {
        p2 p = ps[i];
        while (res.size() > lower && cross(res[res.size() - 2], res.back(), p) <= 0) res.pop_back();
        res.push_back(p);
    }
    res.pop_back();
    return res;
}

}
