#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

namespace cp {

template <class T, class Compare = std::less<T>>
class RMQ {
public:
    using value_type = T;
    using size_type = std::size_t;

    explicit RMQ(std::vector<T> v, Compare comp = Compare{})
        : a(std::move(v)), cmp(std::move(comp)) { build(); }

    RMQ(std::initializer_list<T> v, Compare comp = Compare{})
        : RMQ(std::vector<T>(v), std::move(comp)) {}

    [[nodiscard]] inline size_type size() const noexcept { return a.size(); }

    [[nodiscard]] inline bool empty() const noexcept { return a.empty(); }

    [[nodiscard]] inline const T& query(size_type l, size_type r) const { return a[query_index(l, r)]; }

    [[nodiscard]] inline size_type query_index(size_type l, size_type r) const {
        const size_type last = r - 1;
        const size_type lb = l >> block_shift, rb = last >> block_shift;
        if (lb == rb) return small(last, r - l);
        const size_type end = (lb + 1) << block_shift;
        size_type ans = small(end - 1, end - l);
        if (lb + 1 < rb) ans = best(ans, large(lb + 1, rb));
        return best(ans, small(last, r - (rb << block_shift)));
    }

private:
    using Mask = std::uint64_t;
    static constexpr unsigned block_shift = 6;
    static constexpr size_type block_size = size_type{1} << block_shift;

    static_assert(sizeof(size_type) <= sizeof(Mask), "RMQ requires 64-bit size_t");

    static inline unsigned log2(Mask x) noexcept { return 63U - unsigned(__builtin_clzll(x)); }

    static inline unsigned ctz(Mask x) noexcept { return unsigned(__builtin_ctzll(x)); }

    inline size_type best(size_type i, size_type j) const { return cmp(a[j], a[i]) ? j : i; }

    inline size_type small(size_type last, size_type len) const noexcept {
        Mask mask = masks[last];
        if (len < block_size) mask &= (Mask{1} << len) - 1;
        return last - log2(mask);
    }

    inline size_type large(size_type l, size_type r) const {
        const unsigned k = log2(r - l);
        const size_type len = size_type{1} << k;
        const size_type row = k * blocks;
        return best(sparse[row + l], sparse[row + r - len]);
    }

    inline void build() {
        const size_type n = a.size();
        masks.resize(n);
        blocks = (n + block_size - 1) >> block_shift;
        if (blocks == 0) return;
        const unsigned levels = log2(blocks) + 1;
        sparse.resize(levels * blocks);
        size_type block = 0;
        for (size_type i = 0; i < n; ++i) {
            Mask mask = (i & (block_size - 1)) ? masks[i - 1] << 1 : 0;
            while (mask != 0) {
                const size_type j = i - ctz(mask);
                if (!cmp(a[i], a[j])) break;
                mask &= mask - 1;
            }
            masks[i] = mask | 1;
            if ((i & (block_size - 1)) == block_size - 1 || i + 1 == n) {
                sparse[block++] = small(i, (i & (block_size - 1)) + 1);
            }
        }
        for (unsigned k = 1; k < levels; ++k) {
            const size_type half = size_type{1} << (k - 1), len = half << 1;
            const size_type prev = (k - 1) * blocks;
            const size_type row = k * blocks;
            for (size_type i = 0; i + len <= blocks; ++i) {
                sparse[row + i] = best(sparse[prev + i], sparse[prev + i + half]);
            }
        }
    }

    std::vector<T> a;
    Compare cmp;
    std::vector<Mask> masks;
    std::vector<size_type> sparse;
    size_type blocks = 0;
};

template <class T> RMQ(std::vector<T>) -> RMQ<T>;

template <class T, class Compare> RMQ(std::vector<T>, Compare) -> RMQ<T, Compare>;

}
