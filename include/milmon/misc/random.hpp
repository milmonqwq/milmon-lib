#pragma once

#include <chrono>
#include <random>

namespace cp {

inline std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());

template <class T> inline T rand(T l, T r) { return std::uniform_int_distribution<T>(l, r)(rng); }

}
