#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "milmon/ds/rmq.hpp"

namespace cp {
namespace detail {

inline std::vector<int> sa_naive(const std::vector<int>& s) {
    const int n = int(s.size());
    std::vector<int> sa(n);
    std::iota(sa.begin(), sa.end(), 0);
    std::sort(sa.begin(), sa.end(), [&](int l, int r) {
        while (l < n && r < n) {
            if (s[l] != s[r]) return s[l] < s[r];
            ++l;
            ++r;
        }
        return l == n && r != n;
    });
    return sa;
}

inline std::vector<int> sa_is(const std::vector<int>& s, int upper) {
    const int n = int(s.size());
    if (n < 16) return sa_naive(s);
    std::vector<int> sa(n, -1);
    std::vector<unsigned char> ls(n);
    for (int i = n - 2; i >= 0; --i)
        ls[i] = s[i] == s[i + 1] ? ls[i + 1] : s[i] < s[i + 1];
    std::vector<int> sum_l(upper + 1), sum_s(upper + 1);
    for (int i = 0; i < n; ++i) {
        if (ls[i]) ++sum_l[s[i] + 1];
        else ++sum_s[s[i]];
    }
    for (int i = 0; i <= upper; ++i) {
        sum_s[i] += sum_l[i];
        if (i < upper) sum_l[i + 1] += sum_s[i];
    }
    auto induce = [&](const std::vector<int>& lms) {
        std::fill(sa.begin(), sa.end(), -1);
        std::vector<int> buf = sum_s;
        for (int p : lms) sa[buf[s[p]]++] = p;
        buf = sum_l;
        sa[buf[s[n - 1]]++] = n - 1;
        for (int i = 0; i < n; ++i) {
            const int p = sa[i];
            if (p >= 1 && !ls[p - 1]) sa[buf[s[p - 1]]++] = p - 1;
        }
        buf = sum_l;
        for (int i = n - 1; i >= 0; --i) {
            const int p = sa[i];
            if (p >= 1 && ls[p - 1]) sa[--buf[s[p - 1] + 1]] = p - 1;
        }
    };
    std::vector<int> lms_id(n + 1, -1);
    int m = 0;
    for (int i = 1; i < n; ++i) {
        if (!ls[i - 1] && ls[i]) lms_id[i] = m++;
    }
    std::vector<int> lms;
    lms.reserve(m);
    for (int i = 1; i < n; ++i)
        if (!ls[i - 1] && ls[i]) lms.push_back(i);
    induce(lms);
    if (m != 0) {
        std::vector<int> sorted;
        sorted.reserve(m);
        for (int p : sa)
            if (lms_id[p] != -1) sorted.push_back(p);
        std::vector<int> rec_s(m);
        int rec_upper = 0;
        rec_s[lms_id[sorted[0]]] = 0;
        for (int i = 1; i < m; ++i) {
            int l = sorted[i - 1], r = sorted[i];
            const int lend = lms_id[l] + 1 < m ? lms[lms_id[l] + 1] : n;
            const int rend = lms_id[r] + 1 < m ? lms[lms_id[r] + 1] : n;
            bool same = lend - l == rend - r;
            if (same) {
                while (l < lend && s[l] == s[r]) {
                    ++l;
                    ++r;
                }
                if (l == n || s[l] != s[r]) same = false;
            }
            if (!same) ++rec_upper;
            rec_s[lms_id[sorted[i]]] = rec_upper;
        }
        const std::vector<int> rec_sa = sa_is(rec_s, rec_upper);
        for (int i = 0; i < m; ++i) sorted[i] = lms[rec_sa[i]];
        induce(sorted);
    }
    return sa;
}

inline std::vector<int> string_symbols(const std::string& s) {
    std::vector<int> a(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) a[i] = static_cast<unsigned char>(s[i]);
    return a;
}

inline std::vector<int> ranks_from_sa(const std::vector<int>& sa) {
    std::vector<int> rnk(sa.size());
    int i = 0;
    for (int p : sa) rnk[p] = i++;
    return rnk;
}

inline std::uint32_t sortable_key(int x) { return std::uint32_t(x) ^ (std::uint32_t{1} << 31U); }

inline std::vector<int> normalize_symbols(const std::vector<int>& s, int& upper) {
    const int n = int(s.size());
    if (n == 0) { upper = 0; return {}; }
    std::vector<int> ord(n), tmp(n), res(n);
    std::iota(ord.begin(), ord.end(), 0);
    for (unsigned shift = 0; shift < 32U; shift += 8U) {
        std::array<int, 256> cnt{};
        for (int i : ord) ++cnt[(sortable_key(s[i]) >> shift) & 255U];
        int pos = 0;
        for (int& x : cnt) {
            const int next = pos + x;
            x = pos;
            pos = next;
        }
        for (int i : ord) tmp[cnt[(sortable_key(s[i]) >> shift) & 255U]++] = i;
        ord.swap(tmp);
    }
    upper = 0;
    res[ord[0]] = 0;
    for (int i = 1; i < n; ++i) {
        if (s[ord[i - 1]] != s[ord[i]]) ++upper;
        res[ord[i]] = upper;
    }
    return res;
}

inline void radix_sort_runs(std::vector<std::array<int, 3>>& a, int n) {
    if (a.empty()) return;
    std::vector<std::array<int, 3>> buf(a.size());
    std::vector<int> cnt(n + 1);
    for (int k = 2; k >= 0; --k) {
        std::fill(cnt.begin(), cnt.end(), 0);
        for (const auto& x : a) ++cnt[x[k]];
        int pos = 0;
        for (int& x : cnt) {
            const int next = pos + x;
            x = pos;
            pos = next;
        }
        for (const auto& x : a) buf[cnt[x[k]]++] = x;
        a.swap(buf);
    }
}

}

inline std::vector<int> build_sa(const std::string& s) { return detail::sa_is(detail::string_symbols(s), 255); }

inline std::vector<int> build_sa(std::vector<int> s, int upper) { return detail::sa_is(s, upper); }

inline std::vector<int> build_sa(const std::vector<int>& s) {
    int upper = 0;
    std::vector<int> a = detail::normalize_symbols(s, upper);
    return detail::sa_is(a, upper);
}

inline std::vector<int> build_rnk(std::vector<int> s, int upper) {
    return detail::ranks_from_sa(build_sa(std::move(s), upper));
}

inline std::vector<int> build_rnk(const std::string& s) { return detail::ranks_from_sa(build_sa(s)); }

inline std::vector<int> build_rnk(const std::vector<int>& s) { return detail::ranks_from_sa(build_sa(s)); }

struct SuffixArray {
    std::vector<int> sa;
    std::vector<int> rnk;

    SuffixArray() = default;

    explicit SuffixArray(const std::string& s) { build(s); }

    SuffixArray(std::vector<int> s, int upper) { build(std::move(s), upper); }

    explicit SuffixArray(const std::vector<int>& s) { build(s); }

    inline void build(const std::string& s) { build_encoded(detail::string_symbols(s), 255); }

    inline void build(std::vector<int> s, int upper) { build_encoded(std::move(s), upper); }

    inline void build(const std::vector<int>& s) {
        int upper = 0;
        std::vector<int> a = detail::normalize_symbols(s, upper);
        build_encoded(std::move(a), upper);
    }

    [[nodiscard]] inline int size() const noexcept { return int(sa.size()); }

    [[nodiscard]] inline bool empty() const noexcept { return sa.empty(); }

    [[nodiscard]] inline int lcp(int i, int j) const {
        if (i == j) return size() - i;
        int l = rnk[i], r = rnk[j];
        if (l > r) std::swap(l, r);
        return rmq.query(l + 1, r + 1);
    }

    [[nodiscard]] inline std::vector<std::array<int, 3>> runs() const {
        const int n = size();
        if (n < 2) return {};
        std::vector<int> rev_s(data.rbegin(), data.rend());
        SuffixArray rev;
        rev.build_encoded(std::move(rev_s), upper);
        std::vector<int> inv(n);
        for (int i = 0; i < n; ++i) inv[i] = upper - data[i];
        const std::vector<int> inv_sa = detail::sa_is(inv, upper);
        std::vector<int> inv_rnk(n);
        for (int i = 0; i < n; ++i) inv_rnk[inv_sa[i]] = i;
        std::vector<std::array<int, 3>> res;
        res.reserve(n);
        auto collect = [&](const std::vector<int>& rank) {
            std::vector<int> st;
            st.reserve(n);
            for (int i = n - 1; i >= 0; --i) {
                while (!st.empty() && rank[i] < rank[st.back()]) st.pop_back();
                const int j = st.empty() ? n : st.back();
                st.push_back(i);
                const int p = j - i;
                const int right = j == n ? 0 : lcp(i, j);
                const int left = i == 0 ? 0 : rev.lcp(n - i, n - j);
                if (left != 0 && left <= p && left + right >= p) res.push_back({i - left, j + right, p});
            }
        };
        collect(rnk);
        collect(inv_rnk);
        detail::radix_sort_runs(res, n);
        auto out = res.begin();
        for (const auto& run : res) {
            if (out == res.begin() || (*std::prev(out))[0] != run[0] ||
                (*std::prev(out))[1] != run[1]) {
                *out++ = run;
            }
        }
        res.erase(out, res.end());
        return res;
    }

private:
    inline void build_encoded(std::vector<int> s, int sigma) {
        data = std::move(s);
        upper = sigma;
        sa = detail::sa_is(data, upper);
        const int n = int(sa.size());
        rnk = detail::ranks_from_sa(sa);
        std::vector<int> height(n);
        int h = 0;
        for (int i = 0; i < n; ++i) {
            const int rank = rnk[i];
            if (rank == 0) continue;
            const int j = sa[rank - 1];
            while (i + h < n && j + h < n && data[i + h] == data[j + h]) ++h;
            height[rank] = h;
            if (h != 0) --h;
        }
        rmq = RMQ<int>(std::move(height));
    }

    std::vector<int> data;
    int upper = 0;
    RMQ<int> rmq{std::vector<int>{}};
};

using SA = SuffixArray;

}
