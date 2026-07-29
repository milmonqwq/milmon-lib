#pragma once

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

namespace cp {

template <class T>
class Fenwick {
public:
    using value_type = T;
    using size_type = std::size_t;

    explicit Fenwick(size_type n = 0) : tree(n) {}

    explicit Fenwick(std::vector<T> a) : tree(std::move(a)) {
        for (size_type i = 0; i < tree.size(); ++i) {
            const size_type j = i | (i + 1);
            if (j < tree.size()) tree[j] += tree[i];
        }
    }

    Fenwick(std::initializer_list<T> values) : Fenwick(std::vector<T>(values)) {}

    [[nodiscard]] inline size_type size() const noexcept { return tree.size(); }

    [[nodiscard]] inline bool empty() const noexcept { return tree.empty(); }

    inline void add(size_type i, const T& delta) { for (; i < tree.size(); i |= i + 1) tree[i] += delta; }

    [[nodiscard]] inline T prefix_sum(size_type r) const {
        T sum{};
        while (r != 0) {
            sum += tree[r - 1];
            r &= r - 1;
        }
        return sum;
    }

    [[nodiscard]] inline T query(size_type r) const { return prefix_sum(r); }

private:
    std::vector<T> tree;
};

template <class T> Fenwick(std::vector<T>) -> Fenwick<T>;

}
