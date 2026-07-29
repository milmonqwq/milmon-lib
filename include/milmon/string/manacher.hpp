#pragma once

#include <algorithm>
#include <vector>

namespace cp {

template <class Sequence>
inline std::vector<int> manacher(const Sequence& s) {
    const int n = int(s.size()) * 2 + 1;
    std::vector<int> res(n);
    int l = 0, r = -1;
    for (int i = 0; i < n; ++i) {
        int k = i > r ? 0 : std::min(res[l + r - i], r - i);
        while (i - k - 1 >= 0 && i + k + 1 < n) {
            const int x = i - k - 1, y = i + k + 1;
            if ((x & 1) && s[x >> 1] != s[y >> 1]) break;
            ++k;
        }
        res[i] = k;
        if (i + k > r) { l = i - k; r = i + k; }
    }
    return res;
}

}
