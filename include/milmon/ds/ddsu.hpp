#pragma once

#include <numeric>
#include <vector>

namespace cp {

class DDSU {
public:
    explicit DDSU(int n = 0) : parent(n) { std::iota(parent.begin(), parent.end(), 0); }

    inline void reset(int n) {
        parent.resize(n);
        std::iota(parent.begin(), parent.end(), 0);
    }

    inline int find(int x) {
        int r = x;
        while (parent[r] != r) r = parent[r];
        while (x != r) {
            int p = parent[x];
            parent[x] = r;
            x = p;
        }
        return r;
    }

    inline bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        parent[b] = a;
        return true;
    }

    inline bool same(int a, int b) { return find(a) == find(b); }

private:
    std::vector<int> parent;
};

}
