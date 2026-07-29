#pragma once

#include <utility>
#include <vector>

namespace cp {

template <class Sequence>
inline std::vector<std::pair<int, int>> build_lyndon(const Sequence& s) {
    const int n = int(s.size());
    std::vector<std::pair<int, int>> res;
    int l = 0;
    while (l < n) {
        int i = l, r = l + 1;
        while (r < n && !(s[r] < s[i])) {
            if (s[i] < s[r]) i = l;
            else ++i;
            ++r;
        }
        const int len = r - i;
        while (l <= i) { res.emplace_back(l, l + len); l += len; }
    }
    return res;
}

}
