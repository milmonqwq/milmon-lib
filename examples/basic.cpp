#include <iostream>

#include <milmon/all.hpp>

int main() {
    cp::init_io();
    int vertex_count;
    int edge_count;
    if (!(std::cin >> vertex_count >> edge_count)) return 0;

    cp::DSU components(vertex_count);
    for (int edge = 0; edge < edge_count; ++edge) {
        int from = 0;
        int to = 0;
        std::cin >> from >> to;
        components.unite(from, to);
    }

    std::cout << "size of component containing 0 = " << components.size(0) << '\n';
    std::cout << vertex_count << (cp::is_prime(vertex_count) ? " is prime\n"
                                                             : " is not prime\n");
}
