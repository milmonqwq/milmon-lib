#include <array>
#include <cassert>
#include <climits>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <milmon/all.hpp>

namespace {

void test_basic() {
    static_assert(endl == '\n');
    cp::init_io();
    assert(std::cout.precision() == 10 && std::cerr.precision() == 10);
    assert((std::cout.flags() & std::ios::floatfield) == std::ios::fixed);
    assert((std::cerr.flags() & std::ios::floatfield) == std::ios::fixed);
    cp::init_io(4);
    assert(std::cout.precision() == 4 && std::cerr.precision() == 4);
}

void test_debug() {
    if (false) debug("value = %d\n", 42);
    std::ostringstream output;
    std::streambuf* old = std::cerr.rdbuf(output.rdbuf());
    int value = 42;
    std::vector<std::vector<int>> values{{1, 2}, {3}};
    dbg(value,values,std::vector<int>{4, 5},"a,b");
    std::cerr.rdbuf(old);
    assert(output.str() == "value=42, values=[[1,2],[3]], std::vector<int>{4, 5}=[4,5], \"a,b\"=a,b\n");
}

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
    assert(cp::is_prime(61));
    assert(!cp::is_prime(63));
    assert(!cp::is_prime(341550071728321ULL));
    assert(cp::is_prime(18446744073709551557ULL));
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
}

void test_exgcd() {
    int x = 0, y = 0;
    static_assert(std::is_same_v<decltype(cp::exgcd(1, 1, x, y)), int>);
    assert(cp::exgcd(30, 18, x, y) == 6);
    assert(30 * x + 18 * y == 6);
    assert(cp::exgcd(0, 0, x, y) == 0);
    assert(0 * x + 0 * y == 0);
    assert(cp::exgcd(-35, 15, x, y) == 5);
    assert(-35 * x + 15 * y == 5);
    long long lx = 0, ly = 0;
    constexpr long long a = 4000000007LL, b = 3000000019LL;
    static_assert(std::is_same_v<decltype(cp::exgcd(a, b, lx, ly)), long long>);
    const long long g = cp::exgcd(a, b, lx, ly);
    assert(g == std::gcd(a, b));
    assert(i128(a) * lx + i128(b) * ly == g);
}

void test_floor_sum() {
    assert(cp::floor_sum(0, 7, 3, 4) == 0);
    assert(cp::floor_sum(4, 10, 6, 3) == 3);
    assert(cp::floor_sum(5, 7, -3, 4) == -4);
    assert(cp::floor_sum(3, 5, 2, -4) == -2);
    assert(cp::floor_sum(4000000000LL, 4000000000LL, 3999999999LL,
                         3999999999LL) == 7999999998000000000LL);
}

void test_frac() {
    cp::frac<long long> a{6, -8}, b{5, 6};
    assert(a.num == -3 && a.den == 4);
    assert((cp::frac<int>{0, -7} == cp::frac<int>{0, 1}));
    assert((a + b == cp::frac<long long>{1, 12}));
    assert((a - b == cp::frac<long long>{-19, 12}));
    assert((a * b == cp::frac<long long>{-5, 8}));
    assert((a / b == cp::frac<long long>{-9, 10}));
    assert((-a == cp::frac<long long>{3, 4}));
    assert((2 + a == cp::frac<long long>{5, 4}));
    assert((2 - a == cp::frac<long long>{11, 4}));
    assert((2 * a == cp::frac<long long>{-3, 2}));
    assert((2 / a == cp::frac<long long>{-8, 3}));
    a += 2;
    assert((a == cp::frac<long long>{5, 4}));
    a -= 1;
    a *= 6;
    a /= 3;
    assert((a == cp::frac<long long>{1, 2}));
    assert(a < b && b > a && a <= b && b >= a);
    assert((cp::frac<long long>{4000000001LL, 4000000000LL} <
            cp::frac<long long>{4000000000LL, 3999999999LL}));
    assert((cp::frac<int>{1, 8}.value() == 0.125L));
    const i128 big = i128(1) << 100;
    assert((cp::frac<i128>{big, 3} + cp::frac<i128>{big, 6} == cp::frac<i128>{big / 2}));
    assert((cp::frac<i128>{big, 3} < cp::frac<i128>{big + 1, 3}));
    assert((cp::frac<long long>{-2, 3} < cp::frac<long long>{-3, 5}));
}

void test_rmq() {
    std::vector<int> values(130, 5);
    values[0] = 4;
    values[63] = -10;
    values[64] = -20;
    values[100] = 7;
    values[127] = -30;
    values[128] = -30;
    values[129] = 2;

    cp::RMQ<int> minimum(values);
    assert(minimum.size() == values.size());
    assert(!minimum.empty());
    assert(minimum.query_index(0, 130) == 127);
    assert(minimum.query(10, 100) == -20);
    assert(minimum.query_index(120, 130) == 127);
    assert(minimum.query_index(128, 130) == 128);

    cp::RMQ<int, std::greater<int>> maximum(values, std::greater<int>{});
    assert(maximum.query(10, 120) == 7);

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

    assert(tree.prefix_sum(0) == 0);
    assert(tree.prefix_sum(1) == 3);
    assert(tree.prefix_sum(4) == 7);
    assert(tree.query(7) == 5);

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
    assert(cp::kmp(std::string("ab"), std::string("abc")).empty());
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
    assert((periodic.sa == std::vector<int>{22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0,
                                             23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1}));
    assert((periodic.rnk == std::vector<int>{11, 23, 10, 22, 9, 21, 8, 20, 7, 19, 6, 18,
                                              5, 17, 4, 16, 3, 15, 2, 14, 1, 13, 0, 12}));
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
    test_frac();
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
