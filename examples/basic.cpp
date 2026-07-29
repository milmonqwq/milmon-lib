#include <cstdint>

#include <milmon/all.hpp>

int main() {
    cp::FastScanner input;
    cp::FastOutput output;

    int vertex_count;
    int edge_count;
    if (!input.read(vertex_count) || !input.read(edge_count)) {
        return 0;
    }

    cp::DSU components(vertex_count);
    for (int edge = 0; edge < edge_count; ++edge) {
        int from = 0;
        int to = 0;
        input >> from >> to;
        components.unite(from, to);
    }

    output << "size of component containing 0 = " << components.size(0) << '\n';
    output << vertex_count << (cp::is_prime(vertex_count) ? " is prime\n"
                                                          : " is not prime\n");
}
