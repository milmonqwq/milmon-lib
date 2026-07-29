#include <iostream>

#include <milmon/all.hpp>

// FastScanner in a comment must not pull in fast_io.
const char* ignored_text = R"tag(FastOutput is also not code)tag";

int main() {
    cp::DSU sets(3);
    sets.unite(0, 1);
    std::cout << (sets.same(0, 1) && cp::is_prime(97)) << '\n';
}
