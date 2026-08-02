#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>
#include <functional>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include <milmon/all.hpp>

namespace {

void test_basic() { static_assert(endl == '\n'); cp::init_io(); }

void test_debug() { if (false) debug("value = %d\n", 42); }

void test_dsu() {
    cp::DSU sets(6);
    assert(sets.unite(0, 1));
    assert(sets.unite(1, 2));
    assert(!sets.unite(0, 2));
    assert(sets.same(0, 2));
    assert(!sets.same(0, 3));
    assert(sets.size(1) == 3);

    sets.reset(2);
    assert(sets.size(0) == 1);
}

void test_global_types() {
    static_assert(std::is_same_v<u32, std::uint32_t>);
    static_assert(std::is_same_v<u64, std::uint64_t>);
    static_assert(std::is_same_v<i32, int>);
    static_assert(std::is_same_v<i64, long long>);
    static_assert(std::is_same_v<ll, long long>);
    static_assert(std::is_same_v<ull, u64>);
    static_assert(std::is_same_v<ld, long double>);
    static_assert(std::is_same_v<u128, __uint128_t>);
    static_assert(std::is_same_v<i128, __int128_t>);

    u64 large = 1ULL << 63U;
    u128 square = u128(large) * large;
    assert(u64(square >> 64U) == 1ULL << 62U);
}

void test_primality() {
    assert(!cp::is_prime(-7));
    assert(!cp::is_prime(0));
    assert(!cp::is_prime(1));
    assert(cp::is_prime(2));
    assert(cp::is_prime(97));
    assert(!cp::is_prime(221));
    assert(!cp::is_prime(341550071728321ULL));
    assert(cp::is_prime(18446744073709551557ULL));

    for (int value = 0; value <= 10000; ++value) {
        bool expected = value >= 2;
        for (int divisor = 2; divisor * divisor <= value; ++divisor) {
            if (value % divisor == 0) {
                expected = false;
                break;
            }
        }
        assert(cp::is_prime(value) == expected);
    }
}

void test_pollard_rho() {
    assert(cp::factorize(0).empty());
    assert(cp::factorize(1).empty());
    assert((cp::factorize(360) == std::vector<u64>{2, 2, 2, 3, 3, 5}));
    assert(cp::factorize(1ULL << 63U) == std::vector<u64>(63, 2));

    constexpr u64 first_prime = 1000000007ULL;
    constexpr u64 second_prime = 1000000009ULL;
    constexpr u64 semiprime = first_prime * second_prime;
    const u64 divisor = cp::pollard_rho(semiprime);
    assert(divisor != 1 && divisor != semiprime && semiprime % divisor == 0);
    assert((cp::factorize(semiprime) == std::vector<u64>{first_prime, second_prime}));
    assert((cp::factorize(first_prime * first_prime) ==
            std::vector<u64>{first_prime, first_prime}));

    assert((cp::factorize(18446744073709551615ULL) ==
            std::vector<u64>{3, 5, 17, 257, 641, 65537, 6700417}));
    assert(cp::pollard_rho(first_prime) == first_prime);

    for (u64 value = 2; value <= 2000; ++value) {
        const std::vector<u64> factors = cp::factorize(value);
        u64 product = 1;
        for (u64 factor : factors) {
            assert(cp::is_prime(factor));
            product *= factor;
        }
        assert(product == value);
        assert(std::is_sorted(factors.begin(), factors.end()));
    }
}

void test_exgcd() {
    int x = 0, y = 0;
    static_assert(std::is_same_v<decltype(cp::exgcd(1, 1, x, y)), int>);
    assert(cp::exgcd(30, 18, x, y) == 6);
    assert(30 * x + 18 * y == 6);
    assert(cp::exgcd(0, 0, x, y) == 0);
    assert(0 * x + 0 * y == 0);
    for (int a = -100; a <= 100; ++a) {
        for (int b = -100; b <= 100; ++b) {
            const int g = cp::exgcd(a, b, x, y);
            assert(g == std::gcd(a, b));
            assert(a * x + b * y == g);
        }
    }
    long long lx = 0, ly = 0;
    constexpr long long a = 4000000007LL, b = 3000000019LL;
    static_assert(std::is_same_v<decltype(cp::exgcd(a, b, lx, ly)), long long>);
    const long long g = cp::exgcd(a, b, lx, ly);
    assert(g == std::gcd(a, b));
    assert(i128(a) * lx + i128(b) * ly == g);
}

i64 brute_floor_sum(i64 n, i64 m, i64 a, i64 b) {
    i64 res = 0;
    for (i64 i = 0; i < n; ++i) {
        const i64 v = a * i + b;
        i64 q = v / m;
        if (v % m < 0) --q;
        res += q;
    }
    return res;
}

void test_floor_sum() {
    assert(cp::floor_sum(0, 7, 3, 4) == 0);
    assert(cp::floor_sum(4, 10, 6, 3) == 3);
    assert(cp::floor_sum(5, 7, -3, 4) == -4);
    assert(cp::floor_sum(4000000000LL, 4000000000LL, 3999999999LL,
                         3999999999LL) == 7999999998000000000LL);
    for (i64 n = 0; n <= 20; ++n) {
        for (i64 m = 1; m <= 12; ++m) {
            for (i64 a = -20; a <= 20; ++a) {
                for (i64 b = -20; b <= 20; ++b) {
                    assert(cp::floor_sum(n, m, a, b) == brute_floor_sum(n, m, a, b));
                }
            }
        }
    }
}

void test_fast_io() {
    std::FILE* input_file = std::tmpfile();
    assert(input_file != nullptr);
    const char input_text[] = "-42 hello Q 18446744073709551615";
    assert(std::fwrite(input_text, 1, std::strlen(input_text), input_file) ==
           std::strlen(input_text));
    std::rewind(input_file);

    cp::FastScanner input(input_file);
    int number;
    std::string word;
    char letter;
    unsigned long long maximum;
    assert(input.read(number) && number == -42);
    assert(input.read(word) && word == "hello");
    assert(input.read(letter) && letter == 'Q');
    assert(input.read(maximum) && maximum == ULLONG_MAX);
    assert(!input.read(number));
    std::fclose(input_file);

    std::FILE* output_file = std::tmpfile();
    assert(output_file != nullptr);
    {
        cp::FastOutput output(output_file);
        output << LLONG_MIN << ' ' << 0U << ' ' << "done" << '\n';
    }
    std::rewind(output_file);
    char output_text[128]{};
    const std::size_t length = std::fread(output_text, 1, sizeof(output_text) - 1,
                                          output_file);
    output_text[length] = '\0';
    assert(std::string(output_text) == "-9223372036854775808 0 done\n");
    std::fclose(output_file);
}

void test_rmq() {
    std::vector<int> values(257);
    for (std::size_t index = 0; index < values.size(); ++index) {
        values[index] = int((index * 97 + index * index * 13) % 101);
    }
    values[63] = -10;
    values[64] = -20;
    values[127] = -30;
    values[128] = -30;

    cp::RMQ<int> minimum(values);
    assert(minimum.size() == values.size());
    assert(!minimum.empty());
    for (std::size_t left = 0; left < values.size(); ++left) {
        std::size_t expected = left;
        for (std::size_t right = left + 1; right <= values.size(); ++right) {
            if (values[right - 1] < values[expected]) {
                expected = right - 1;
            }
            assert(minimum.query_index(left, right) == expected);
            assert(minimum.query(left, right) == values[expected]);
        }
    }

    cp::RMQ<int, std::greater<int>> maximum(values, std::greater<int>{});
    assert(maximum.query(10, 200) ==
           *std::max_element(values.begin() + 10, values.begin() + 200));

    cp::RMQ<int> ties({4, 1, 1, 3});
    assert(ties.query_index(0, 4) == 1);
    assert(ties.query_index(2, 4) == 2);

    cp::RMQ<int> empty(std::vector<int>{});
    assert(empty.empty());
}

void test_fenwick() {
    const std::vector<i64> values{3, -1, 4, 1, 5, -9, 2};
    cp::Fenwick<i64> tree(values);
    assert(tree.size() == values.size());
    assert(!tree.empty());

    i64 expected = 0;
    assert(tree.prefix_sum(0) == 0);
    for (std::size_t right = 1; right <= values.size(); ++right) {
        expected += values[right - 1];
        assert(tree.prefix_sum(right) == expected);
        assert(tree.query(right) == expected);
    }

    tree.add(1, 10);
    tree.add(6, -2);
    assert(tree.prefix_sum(1) == 3);
    assert(tree.prefix_sum(2) == 12);
    assert(tree.prefix_sum(7) == 13);

    cp::Fenwick<int> zeroes(4);
    zeroes.add(3, 7);
    assert(zeroes.prefix_sum(3) == 0);
    assert(zeroes.prefix_sum(4) == 7);
}

void test_p2() {
    cp::p2 a{2, -3}, b{-4, 5};
    assert(a + b == (cp::p2{-2, 2}));
    assert(a - b == (cp::p2{6, -8}));
    assert(-a == (cp::p2{-2, 3}));
    assert(3 * a == (cp::p2{6, -9}));
    assert(cp::dot(a, b) == -23);
    assert(cp::cross(a, b) == -2);
    assert(cp::cross(cp::p2{1, 1}, cp::p2{3, 1}, cp::p2{2, 4}) == 6);
    assert(cp::cross(cp::p2{INT_MIN, 0}, cp::p2{INT_MAX, 0},
                     cp::p2{INT_MIN, 1}) == 4294967295LL);
    a += b;
    assert(a == (cp::p2{-2, 2}));
    a -= b;
    a *= 2;
    assert(a == (cp::p2{4, -6}));
}

void test_p2r() {
    cp::p2r<double> a{1.5, -2.0}, b{-0.5, 4.0};
    assert(a + b == (cp::p2r<double>{1.0, 2.0}));
    assert(a - b == (cp::p2r<double>{2.0, -6.0}));
    assert(-a == (cp::p2r<double>{-1.5, 2.0}));
    assert(2 * a == (cp::p2r<double>{3.0, -4.0}));
    assert(a / 2 == (cp::p2r<double>{0.75, -1.0}));
    assert(cp::dot(a, b) == -8.75);
    assert(cp::cross(a, b) == 5.0);
    assert(cp::cross(cp::p2r<double>{1.0, 1.0}, cp::p2r<double>{2.5, 1.0},
                     cp::p2r<double>{1.0, 3.0}) == 3.0);
    a += b;
    a -= b;
    a *= 2;
    a /= 4;
    assert(a == (cp::p2r<double>{0.75, -1.0}));
    cp::p2r<ld> c{1.0L, 2.0L}, d{3.0L, -1.0L};
    static_assert(std::is_same_v<decltype(cp::dot(c, d)), ld>);
    assert(cp::dot(c, d) == 1.0L);
    assert(cp::cross(c, d) == -7.0L);
}

void test_convex_hull() {
    assert(cp::convex_hull({}).empty());
    assert((cp::convex_hull({{2, 3}, {2, 3}}) == std::vector<cp::p2>{{2, 3}}));
    assert((cp::convex_hull({{0, 0}, {1, 0}, {2, 0}, {1, 0}}) ==
            std::vector<cp::p2>{{0, 0}, {2, 0}}));
    std::vector<cp::p2> ps{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 0},
                           {2, 1}, {1, 2}, {0, 1}, {1, 1}, {0, 0}};
    assert((cp::convex_hull(ps) ==
            std::vector<cp::p2>{{0, 0}, {2, 0}, {2, 2}, {0, 2}}));
    const std::vector<cp::p2> grid{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                                   {0, 1},   {1, -1}, {1, 0},  {1, 1}};
    for (int mask = 0; mask < (1 << int(grid.size())); ++mask) {
        std::vector<cp::p2> points;
        for (int i = 0; i < int(grid.size()); ++i) {
            if (mask >> i & 1) points.push_back(grid[i]);
        }
        const std::vector<cp::p2> hull = cp::convex_hull(points);
        if (hull.empty()) { assert(points.empty()); continue; }
        assert(hull.front() == *std::min_element(points.begin(), points.end()));
        if (hull.size() == 1) { assert(points.size() == 1); continue; }
        if (hull.size() == 2) {
            for (cp::p2 p : points) assert(cp::cross(hull[0], hull[1], p) == 0);
            continue;
        }
        for (std::size_t i = 0; i < hull.size(); ++i) {
            cp::p2 a = hull[i], b = hull[(i + 1) % hull.size()];
            assert(cp::cross(hull[(i + hull.size() - 1) % hull.size()], a, b) > 0);
            for (cp::p2 p : points) assert(cp::cross(a, b, p) >= 0);
        }
    }
}

void test_kmp() {
    assert((cp::prefix_function(std::string("ababaca")) ==
            std::vector<int>{0, 0, 1, 2, 3, 0, 1}));
    assert((cp::kmp(std::vector<int>{1, 2, 1, 2, 1}) ==
            std::vector<int>{0, 0, 1, 2, 3}));
    assert((cp::kmp(std::string("aaaaa"), std::string("aa")) ==
            std::vector<int>{0, 1, 2, 3}));
    assert((cp::kmp(std::vector<int>{1, 2, 1, 2, 1},
                    std::vector<int>{1, 2, 1}) == std::vector<int>{0, 2}));
    assert((cp::kmp(std::string("abc"), std::string("")) ==
            std::vector<int>{0, 1, 2, 3}));
}

void test_z_function() {
    assert((cp::z_function(std::string("aabcaabxaaaz")) ==
            std::vector<int>{0, 1, 0, 0, 3, 1, 0, 0, 2, 2, 1, 0}));
    assert((cp::z_function(std::vector<int>{1, 2, 1, 2, 1}) ==
            std::vector<int>{0, 0, 3, 0, 1}));
    assert(cp::z_function(std::string()).empty());
}

void test_manacher() {
    assert((cp::manacher(std::string("abacaba")) ==
            std::vector<int>{0, 1, 0, 3, 0, 1, 0, 7, 0, 1, 0, 3, 0, 1, 0}));
    assert((cp::manacher(std::string("abba")) ==
            std::vector<int>{0, 1, 0, 1, 4, 1, 0, 1, 0}));
    assert(cp::manacher(std::vector<int>{1, 2, 1, 1, 2, 1})[6] == 6);
    assert((cp::manacher(std::string()) == std::vector<int>{0}));
}

void test_lyndon() {
    assert((cp::build_lyndon(std::string("banana")) ==
            std::vector<std::pair<int, int>>{{0, 1}, {1, 3}, {3, 5}, {5, 6}}));
    assert((cp::build_lyndon(std::string("ababbab")) ==
            std::vector<std::pair<int, int>>{{0, 5}, {5, 7}}));
    assert((cp::build_lyndon(std::vector<int>{2, 1, 2, 1}) ==
            std::vector<std::pair<int, int>>{{0, 1}, {1, 3}, {3, 4}}));
    assert(cp::build_lyndon(std::string()).empty());
}

void test_suffix_array() {
    assert((cp::build_sa(std::string("banana")) ==
            std::vector<int>{5, 3, 1, 0, 4, 2}));
    assert((cp::build_rnk(std::string("banana")) ==
            std::vector<int>{3, 2, 5, 1, 4, 0}));
    assert((cp::build_sa(std::vector<int>{3, -1, 3, 0}) ==
            std::vector<int>{1, 3, 0, 2}));
    assert((cp::build_rnk(std::vector<int>{2, 0, 2, 1, 0}, 2) ==
            std::vector<int>{3, 1, 4, 2, 0}));

    cp::SA empty("");
    assert(empty.sa.empty());
    assert(empty.rnk.empty());
    assert(empty.runs().empty());

    cp::SA singleton("x");
    assert((singleton.sa == std::vector<int>{0}));
    assert((singleton.rnk == std::vector<int>{0}));
    assert(singleton.lcp(0, 0) == 1);
    assert(singleton.runs().empty());

    cp::SA pair("aa");
    assert((pair.sa == std::vector<int>{1, 0}));
    assert((pair.rnk == std::vector<int>{1, 0}));
    assert(pair.lcp(0, 1) == 1);
    assert((pair.runs() == std::vector<std::array<int, 3>>{{0, 2, 1}}));

    cp::SA banana("banana");
    assert((banana.sa == std::vector<int>{5, 3, 1, 0, 4, 2}));
    assert((banana.rnk == std::vector<int>{3, 2, 5, 1, 4, 0}));
    assert(banana.lcp(1, 3) == 3);
    assert(banana.lcp(2, 4) == 2);
    assert(banana.lcp(0, 0) == 6);

    const std::vector<int> raw_values{3, -1, 3, 0};
    cp::SA raw_suffixes(raw_values);
    assert((raw_suffixes.sa == std::vector<int>{1, 3, 0, 2}));
    assert((raw_suffixes.rnk == std::vector<int>{2, 0, 3, 1}));

    cp::SuffixArray dense(std::vector<int>{2, 0, 2, 1, 0}, 2);
    assert((dense.sa == std::vector<int>{4, 1, 3, 0, 2}));

    cp::SuffixArray periodic("abababababababababababab");
    for (int rank = 0; rank < 12; ++rank) {
        assert(periodic.sa[rank] == 22 - rank * 2);
        assert(periodic.sa[rank + 12] == 23 - rank * 2);
        assert(periodic.rnk[periodic.sa[rank]] == rank);
        assert(periodic.rnk[periodic.sa[rank + 12]] == rank + 12);
    }
    assert(periodic.lcp(0, 2) == 22);
    assert((periodic.runs() ==
            std::vector<std::array<int, 3>>{{0, 24, 2}}));

    cp::SuffixArray run_example("aababaababb");
    assert((run_example.runs() ==
            std::vector<std::array<int, 3>>{{0, 2, 1}, {0, 10, 5}, {1, 6, 2},
                                             {3, 9, 3}, {5, 7, 1}, {6, 10, 2},
                                             {9, 11, 1}}));
}

}

int main() {
    test_basic();
    test_debug();
    test_global_types();
    test_dsu();
    test_primality();
    test_pollard_rho();
    test_exgcd();
    test_floor_sum();
    test_fast_io();
    test_rmq();
    test_fenwick();
    test_p2();
    test_p2r();
    test_convex_hull();
    test_kmp();
    test_z_function();
    test_manacher();
    test_lyndon();
    test_suffix_array();
}
