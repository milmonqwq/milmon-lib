#pragma once

#include <vector>

namespace cp {

template <class Sequence>
inline std::vector<int> prefix_function(const Sequence& s) {
    const int n = int(s.size());
    std::vector<int> pi(n);
    for (int i = 1; i < n; ++i) {
        int j = pi[i - 1];
        while (j != 0 && s[i] != s[j]) j = pi[j - 1];
        if (s[i] == s[j]) ++j;
        pi[i] = j;
    }
    return pi;
}

template <class Sequence>
inline std::vector<int> kmp(const Sequence& s) { return prefix_function(s); }

template <class Text, class Pattern>
inline std::vector<int> kmp(const Text& text, const Pattern& pattern) {
    const int n = int(text.size());
    const int m = int(pattern.size());
    std::vector<int> pos;
    if (m == 0) {
        pos.resize(n + 1);
        for (int i = 0; i <= n; ++i) pos[i] = i;
        return pos;
    }
    if (m > n) return pos;
    const std::vector<int> pi = prefix_function(pattern);
    int j = 0;
    for (int i = 0; i < n; ++i) {
        while (j != 0 && text[i] != pattern[j]) j = pi[j - 1];
        if (text[i] == pattern[j]) ++j;
        if (j == m) { pos.push_back(i - m + 1); j = pi[j - 1]; }
    }
    return pos;
}

}
