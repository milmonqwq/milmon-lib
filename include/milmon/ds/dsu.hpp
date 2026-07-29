#pragma once

#include <utility>
#include <vector>

namespace cp {

class DSU {
public:
    explicit DSU(int n = 0) : val(n, -1) {}

    inline void reset(int n) { val.assign(n, -1); }

    inline int find(int x) {
        while (val[x] >= 0) {
            if (val[val[x]] >= 0) val[x] = val[val[x]];
            x = val[x];
        }
        return x;
    }

    inline bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (val[a] > val[b]) std::swap(a, b);
        val[a] += val[b];
        val[b] = a;
        return true;
    }

    inline bool same(int a, int b) { return find(a) == find(b); }

    inline int size(int x) { return -val[find(x)]; }

private:
    std::vector<int> val;
};

}
