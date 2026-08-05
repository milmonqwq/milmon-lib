#pragma once

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>

#define endl '\n'

namespace cp {

inline void init_io(int precision = 10) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr); std::cout.tie(nullptr);
    std::cout << std::fixed << std::setprecision(precision);
    std::cerr << std::fixed << std::setprecision(precision);
}

inline void file_io(const std::string& name) {
    std::freopen((name + ".in").c_str(), "r", stdin);
    std::freopen((name + ".out").c_str(), "w", stdout);
}

}
