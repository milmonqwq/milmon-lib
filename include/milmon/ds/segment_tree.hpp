#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

namespace cp {

template <class T, class Compare = std::less<T>>
class SegmentTree {
public:
    using value_type = T;
    using size_type = std::size_t;
    std::vector<T> a;

    explicit SegmentTree(size_type n = 0, Compare comp = Compare{})
        : SegmentTree(std::vector<T>(n), std::move(comp)) {}

    SegmentTree(size_type n, const T& value, Compare comp = Compare{})
        : SegmentTree(std::vector<T>(n, value), std::move(comp)) {}

    explicit SegmentTree(std::vector<T> values, Compare comp = Compare{})
        : a(std::move(values)), cmp(std::move(comp)), tree(2 * a.size()) {
        const size_type n = a.size();
        for (size_type i = 0; i < n; ++i) tree[n + i] = i;
        for (size_type i = n; i > 1;) pull(--i);
    }

    SegmentTree(std::initializer_list<T> values, Compare comp = Compare{})
        : SegmentTree(std::vector<T>(values), std::move(comp)) {}

    [[nodiscard]] inline size_type size() const noexcept { return a.size(); }

    [[nodiscard]] inline bool empty() const noexcept { return a.empty(); }

    inline void set(size_type i, const T& value) {
        a[i] = value;
        update(i);
    }

    inline void add(size_type i, const T& delta) {
        a[i] += delta;
        update(i);
    }

    [[nodiscard]] inline const T& query(size_type l, size_type r) const { return a[query_index(l, r)]; }

    [[nodiscard]] inline size_type query_index(size_type l, size_type r) const {
        const size_type n = a.size();
        size_type res = n;
        for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                const size_type i = tree[l++];
                res = res == n ? i : best(res, i);
            }
            if (r & 1) {
                const size_type i = tree[--r];
                res = res == n ? i : best(res, i);
            }
        }
        return res;
    }

    [[nodiscard]] inline const T& query() const { return a[query_index()]; }

    [[nodiscard]] inline size_type query_index() const { return tree[1]; }

private:
    inline size_type best(size_type i, size_type j) const {
        if (i > j) std::swap(i, j);
        return cmp(a[j], a[i]) ? j : i;
    }

    inline void pull(size_type i) { tree[i] = best(tree[i << 1], tree[i << 1 | 1]); }

    inline void update(size_type i) { for (i = (i + a.size()) >> 1; i != 0; i >>= 1) pull(i); }

    Compare cmp;
    std::vector<size_type> tree;
};

template <class T> SegmentTree(std::vector<T>) -> SegmentTree<T>;

template <class T, class Compare> SegmentTree(std::vector<T>, Compare) -> SegmentTree<T, Compare>;

template <class T> SegmentTree(std::size_t, const T&) -> SegmentTree<T>;

template <class T, class Compare> SegmentTree(std::size_t, const T&, Compare) -> SegmentTree<T, Compare>;

}
