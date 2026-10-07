#include <array>
#include <cassert>
#include <climits>
#include <functional>
#include <limits>
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

void test_ddsu() {
    cp::DDSU sets(6);
    assert(sets.find(0) == 0);
    assert(!sets.same(0, 1));
    assert(sets.unite(1, 2));
    assert(sets.unite(1, 3));
    assert(sets.unite(0, 2));
    assert(sets.find(3) == 0);
    assert(sets.unite(4, 0));
    assert(sets.unite(5, 4));
    assert(sets.find(3) == 5);
    assert(sets.same(2, 5));
    assert(!sets.unite(3, 1));
    assert(!sets.unite(5, 5));
    assert(sets.find(1) == 5);

    sets.reset(2);
    assert(sets.find(0) == 0);
    assert(sets.find(1) == 1);
    assert(!sets.same(0, 1));
    sets.reset(0);
    sets.reset(3);
    assert(sets.find(2) == 2);

    cp::DDSU empty;
    empty.reset(1);
    assert(empty.find(0) == 0);
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

void test_splitmix64() {
    static_assert(cp::splitmix64(0) == 0xe220a8397b1dcdafULL);
    assert(cp::splitmix64(1) == 0x910a2dec89025cc1ULL);
    assert(cp::splitmix64(std::numeric_limits<u64>::max()) == 0xe4d971771b652c20ULL);
}

void test_random() {
    cp::rng.seed(5489);
    assert(cp::rng() == 14514284786278117030ULL);
    assert(cp::rand(INT_MIN, INT_MIN) == INT_MIN);
    assert(cp::rand(LLONG_MAX, LLONG_MAX) == LLONG_MAX);
    const u64 max = std::numeric_limits<u64>::max();
    assert(cp::rand(max, max) == max);
    const int negative = cp::rand(-9, -2);
    assert(-9 <= negative && negative <= -2);
    const i64 wide = cp::rand(-4000000000000LL, 4000000000000LL);
    assert(-4000000000000LL <= wide && wide <= 4000000000000LL);
    const u64 large = cp::rand(u64{1} << 63U, max);
    assert(large >= (u64{1} << 63U));
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

void test_pow() {
    static_assert(std::is_same_v<decltype(cp::pow<1000000007>(1, 1)), u32>);
    assert(cp::pow<1000000007>(2, 0) == 1);
    assert(cp::pow<1000000007>(2, 10) == 1024);
    assert(cp::pow<998244353>(998244354, 3) == 1);
    assert(cp::inv<1000000007>(2) == 500000004);
    assert(cp::inv<998244353>(3) == 332748118);
}

void test_pollard_rho() {
    assert(cp::factorize(0).empty());
    assert(cp::factorize(1).empty());
    assert((cp::factorize(360) == std::vector<u64>{2, 2, 2, 3, 3, 5}));
    assert(cp::factorize(1ULL << 63U) == std::vector<u64>(63, 2));
    assert(cp::factorize_pair(0).empty());
    assert(cp::factorize_pair(1).empty());
    assert((cp::factorize_pair(360) ==
            std::vector<std::pair<u64, int>>{{2, 3}, {3, 2}, {5, 1}}));
    assert((cp::factorize_pair(1ULL << 63U) == std::vector<std::pair<u64, int>>{{2, 63}}));

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

void test_ex_floor_sum() {
    static_assert(std::is_same_v<decltype(cp::ex_floor_sum(1, 1, 1, 1)),
                                 std::tuple<i128, i128, i128>>);
    assert((cp::ex_floor_sum(0, 7, 3, 4) == std::tuple<i128, i128, i128>{0, 0, 0}));
    assert((cp::ex_floor_sum(4, 10, 6, 3) == std::tuple<i128, i128, i128>{3, 8, 5}));
    assert((cp::ex_floor_sum(5, 7, -3, 4) == std::tuple<i128, i128, i128>{-4, -13, 6}));
    assert((cp::ex_floor_sum(3, 5, 2, -4) == std::tuple<i128, i128, i128>{-2, -1, 2}));
    assert((cp::ex_floor_sum(5, 3, 8, 7) == std::tuple<i128, i128, i128>{37, 101, 347}));
    assert((cp::ex_floor_sum(2000000, 2000000, 2000000, 0) ==
            std::tuple<i128, i128, i128>{1999999000000LL, 2666664666667000000LL,
                                         2666664666667000000LL}));
    auto [s0, s1, s2] = cp::ex_floor_sum(100000, 1, 100000, 0);
    assert(s0 == i128(499995000000000LL));
    assert(s1 == i128(33332833335LL) * 1000000000LL);
    assert(s2 == i128(33332833335LL) * 100000000000000LL);
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

void test_poly_mul() {
    cp::poly::init();
    cp::poly::init();
    assert(cp::poly::poly_mul({}, {1, 2}).empty());
    assert((cp::poly::poly_mul({7}, {9}) == std::vector<u32>{63}));
    assert((cp::poly::poly_mul({1, 2, 3}, {4, 5, 6}) ==
            std::vector<u32>{4, 13, 28, 27, 18}));
    assert((cp::poly::poly_mul({998244352U, 1}, {1, 1}) ==
            std::vector<u32>{998244352U, 0, 1}));
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

void test_segment_tree() {
    const std::vector<i64> values{3, -1, 4, 4, 5, -9, 2};
    cp::SegmentTree tree(values);
    assert(tree.size() == 7 && !tree.empty());
    assert(tree.a == values);
    assert(tree.query() == -9 && tree.query_index() == 5);
    assert(tree.query(0, 7) == -9 && tree.query_index(0, 7) == 5);
    assert(tree.query(0, 5) == -1 && tree.query_index(0, 5) == 1);
    assert(tree.query(2, 4) == 4 && tree.query_index(2, 4) == 2);
    assert(tree.query(6, 7) == 2 && tree.query_index(6, 7) == 6);

    tree.set(5, 8);
    assert(tree.a[5] == 8 && tree.query() == -1 && tree.query_index() == 1);
    assert(tree.query(4, 7) == 2 && tree.query_index(4, 7) == 6);
    tree.add(1, 10);
    assert(tree.a[1] == 9 && tree.query() == 2 && tree.query_index() == 6);
    tree.set(6, -5);
    tree.set(3, -5);
    assert(tree.query() == -5 && tree.query_index() == 3);
    assert(tree.query(4, 7) == -5 && tree.query_index(4, 7) == 6);
    tree.set(0, -5);
    assert(tree.query_index() == 0);
    assert(tree.query_index(1, 7) == 3);

    cp::SegmentTree<int> zeroes(5);
    assert(zeroes.query() == 0 && zeroes.query_index() == 0);
    assert(zeroes.query_index(1, 5) == 1);
    zeroes.add(4, -7);
    assert(zeroes.query() == -7 && zeroes.query_index() == 4);
    zeroes.set(4, 0);
    assert(zeroes.query_index() == 0);

    cp::SegmentTree filled(5, 7);
    assert(filled.size() == 5 && filled.query() == 7 && filled.query_index() == 0);
    assert(filled.query(3, 5) == 7 && filled.query_index(3, 5) == 3);
    filled.set(4, -2);
    assert(filled.query() == -2 && filled.query_index() == 4);
    filled.add(4, 9);
    assert(filled.query() == 7 && filled.query_index() == 0);

    cp::SegmentTree<int> singleton{6};
    singleton.set(0, -3);
    singleton.add(0, 2);
    assert(singleton.query() == -1 && singleton.query_index() == 0);
    assert(singleton.query(0, 1) == -1 && singleton.query_index(0, 1) == 0);

    cp::SegmentTree<int> extremes{INT_MAX, INT_MIN, INT_MAX};
    assert(extremes.query() == INT_MIN && extremes.query_index() == 1);
    assert(extremes.query(2, 3) == INT_MAX && extremes.query_index(2, 3) == 2);

    const auto closer = [pivot = 10](int l, int r) {
        const int a = l < pivot ? pivot - l : l - pivot;
        const int b = r < pivot ? pivot - r : r - pivot;
        return a < b;
    };
    cp::SegmentTree nearest(std::vector<int>{7, 13, 20, 9, 11}, closer);
    assert(nearest.query() == 9 && nearest.query_index() == 3);
    assert(nearest.query(1, 3) == 13 && nearest.query_index(1, 3) == 1);
    nearest.set(0, 11);
    assert(nearest.query() == 11 && nearest.query_index() == 0);
    assert(nearest.query(1, 5) == 9 && nearest.query_index(1, 5) == 3);
    nearest.add(3, 1);
    assert(nearest.query() == 10 && nearest.query_index() == 3);
    nearest.set(3, 30);
    assert(nearest.query() == 11 && nearest.query_index() == 0);

    cp::SegmentTree uniform_nearest(5, 13, closer);
    assert(uniform_nearest.query() == 13 && uniform_nearest.query_index() == 0);
    uniform_nearest.set(2, 9);
    assert(uniform_nearest.query() == 9 && uniform_nearest.query_index() == 2);
    uniform_nearest.add(2, 1);
    assert(uniform_nearest.query() == 10 && uniform_nearest.query_index() == 2);

    const auto decade_less = +[](const int& l, const int& r) { return l / 10 < r / 10; };
    cp::SegmentTree<int, decltype(decade_less)> buckets({19, 11, 25}, decade_less);
    assert(buckets.query() == 19 && buckets.query_index() == 0);
    assert(buckets.query(1, 3) == 11 && buckets.query_index(1, 3) == 1);
    buckets.set(2, 10);
    assert(buckets.query_index() == 0);

    cp::SegmentTree<std::string> words{"z", "a", "m"};
    words.set(1, "zz");
    assert(words.query() == "m" && words.query_index() == 2);
    cp::SegmentTree filled_words(3, std::string("seed"));
    assert(filled_words.query(1, 3) == "seed" && filled_words.query_index(1, 3) == 1);
    filled_words.set(2, "apple");
    assert(filled_words.query() == "apple" && filled_words.query_index() == 2);

    cp::SegmentTree<int> empty;
    cp::SegmentTree<int> empty_values(std::vector<int>{});
    cp::SegmentTree<int> empty_filled(0, 7);
    assert(empty.empty() && empty.size() == 0);
    assert(empty_values.empty() && empty_values.size() == 0);
    assert(empty_filled.empty() && empty_filled.size() == 0);
}

void test_max_flow() {
    // 菱形网络（ACL 示例）
    cp::MaxFlow<int> mf(4, 5);
    assert(mf.add_edge(0, 1, 1) == 0);
    assert(mf.add_edge(0, 2, 1) == 1);
    assert(mf.add_edge(1, 3, 1) == 2);
    assert(mf.add_edge(2, 3, 1) == 3);
    assert(mf.add_edge(1, 2, 1) == 4);
    assert(mf.flow(0, 3) == 2);
    assert(mf.flow(0, 3) == 0);

    const cp::MaxFlow<int>::FlowEdge e0 = mf.get_edge(0);
    assert(e0.from == 0 && e0.to == 1 && e0.cap == 1 && e0.flow == 1);
    const std::vector<cp::MaxFlow<int>::FlowEdge> all = mf.edges();
    assert(all.size() == 5);
    assert(all[1].from == 0 && all[1].to == 2 && all[1].cap == 1 && all[1].flow == 1);
    assert(all[3].from == 2 && all[3].to == 3 && all[3].cap == 1 && all[3].flow == 1);
    assert(all[4].from == 1 && all[4].to == 2 && all[4].cap == 1 && all[4].flow == 0);
    assert((mf.min_cut(0) == std::vector<bool>{true, false, false, false}));
    assert((mf.min_cut(3) == std::vector<bool>{true, true, true, true}));

    // 限制最大流量
    cp::MaxFlow<int> limited(4);
    limited.add_edge(0, 1, 1);
    limited.add_edge(0, 2, 1);
    limited.add_edge(1, 3, 1);
    limited.add_edge(2, 3, 1);
    limited.add_edge(1, 2, 1);
    assert(limited.flow(0, 3, 0) == 0);
    assert(limited.flow(0, 3, 1) == 1);
    assert(limited.flow(0, 3) == 1);

    // 需要退流的网络（最大流为 3）
    cp::MaxFlow<int> reroute(4);
    reroute.add_edge(0, 1, 2);
    reroute.add_edge(1, 2, 2);
    reroute.add_edge(2, 3, 2);
    reroute.add_edge(0, 2, 1);
    reroute.add_edge(1, 3, 1);
    assert(reroute.flow(0, 3) == 3);
    assert((reroute.min_cut(0) == std::vector<bool>{true, false, false, false}));

    // 混合网络（ACL 单元测试用例）
    cp::MaxFlow<int> network(6);
    network.add_edge(0, 1, 3);
    network.add_edge(0, 2, 3);
    network.add_edge(1, 2, 2);
    network.add_edge(1, 3, 3);
    network.add_edge(2, 4, 2);
    network.add_edge(3, 4, 4);
    network.add_edge(3, 5, 2);
    network.add_edge(4, 5, 3);
    assert(network.flow(0, 5) == 5);
    const std::vector<bool> network_cut = network.min_cut(0);
    assert(network_cut[0] && !network_cut[5]);
    int network_cut_cap = 0;
    for (const auto& e : network.edges())
        if (network_cut[e.from] && !network_cut[e.to]) network_cut_cap += e.cap;
    assert(network_cut_cap == 5);

    // 长链网络（路径长度 99999，验证手工栈 DFS）
    constexpr int kChain = 100000;
    cp::MaxFlow<int> chain(kChain);
    for (int i = 0; i < kChain - 1; ++i) chain.add_edge(i, i + 1, kChain);
    assert(chain.flow(0, kChain - 1) == kChain);

    // 不连通的点
    cp::MaxFlow<int> disconnected(3);
    disconnected.add_edge(0, 1, 7);
    assert(disconnected.flow(1, 0) == 0);
    assert(disconnected.flow(0, 2) == 0);
    assert((disconnected.min_cut(0) == std::vector<bool>{true, true, false}));

    // 修改边的容量与流量
    cp::MaxFlow<int> dynamic(2);
    dynamic.add_edge(0, 1, 5);
    assert(dynamic.flow(0, 1) == 5);
    dynamic.change_edge(0, 3, 1);
    assert(dynamic.get_edge(0).cap == 3 && dynamic.get_edge(0).flow == 1);
    assert(dynamic.flow(0, 1) == 2);
    assert(dynamic.get_edge(0).cap == 3 && dynamic.get_edge(0).flow == 3);

    // 自环不应破坏反向边索引
    cp::MaxFlow<int> self_loop(2);
    assert(self_loop.add_edge(0, 0, 7) == 0);
    self_loop.change_edge(0, 5, 2);
    assert(self_loop.get_edge(0).from == 0 && self_loop.get_edge(0).to == 0);
    assert(self_loop.get_edge(0).cap == 5 && self_loop.get_edge(0).flow == 2);
    self_loop.add_edge(0, 1, 4);
    assert(self_loop.flow(0, 1) == 4);

    // 浮点流量
    cp::MaxFlow<double> real(3);
    real.add_edge(0, 1, 1.5);
    real.add_edge(1, 2, 0.5);
    real.add_edge(0, 2, 2.0);
    assert(real.flow(0, 2) == 2.5);

    // 64 位大容量
    cp::MaxFlow<i64> big(2);
    big.add_edge(0, 1, 4000000000000LL);
    assert(big.flow(0, 1) == 4000000000000LL);
    assert(big.get_edge(0).flow == 4000000000000LL);
}

void test_min_cost_flow() {
    cp::MinCostFlow<int, i64> mf(6, 8);
    assert(mf.add_edge(0, 1, 1, 0) == 0);
    assert(mf.add_edge(0, 2, 1, 0) == 1);
    assert(mf.add_edge(1, 3, 1, 1) == 2);
    assert(mf.add_edge(1, 4, 1, 3) == 3);
    assert(mf.add_edge(2, 3, 1, 2) == 4);
    assert(mf.add_edge(2, 4, 1, 100) == 5);
    assert(mf.add_edge(3, 5, 1, 0) == 6);
    assert(mf.add_edge(4, 5, 1, 0) == 7);
    assert((mf.flow(0, 5, 0) == std::pair<int, i64>{0, 0}));
    assert((mf.flow(0, 5, 1) == std::pair<int, i64>{1, 1}));
    assert((mf.flow(0, 5) == std::pair<int, i64>{1, 4}));
    assert((mf.flow(0, 5) == std::pair<int, i64>{0, 0}));
    const auto rerouted = mf.get_edge(2);
    assert(rerouted.from == 1 && rerouted.to == 3 && rerouted.cap == 1);
    assert(rerouted.flow == 0 && rerouted.cost == 1);
    const auto all = mf.edges();
    assert(all[3].flow == 1 && all[4].flow == 1 && all[5].flow == 0);

    cp::MinCostFlow<int, i64> negative(3);
    negative.add_edge(0, 1, 1, -4);
    negative.add_edge(1, 2, 1, 3);
    negative.add_edge(0, 2, 1, 0);
    assert((negative.flow(0, 2, 1) == std::pair<int, i64>{1, -1}));
    assert((negative.flow(0, 2, 1) == std::pair<int, i64>{1, 0}));

    cp::MinCostFlow<int, i64> disconnected(3);
    disconnected.add_edge(0, 0, 2, 7);
    disconnected.add_edge(0, 1, 3, 5);
    assert((disconnected.flow(0, 2) == std::pair<int, i64>{0, 0}));
    assert(disconnected.get_edge(0).flow == 0);

    cp::MinCostFlow<i64, i64> big(2);
    big.add_edge(0, 1, 1000000LL, 4000000000000LL);
    assert((big.flow(0, 1) == std::pair<i64, i64>{1000000LL, 4000000000000000000LL}));
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
    test_splitmix64();
    test_random();
    test_dsu();
    test_ddsu();
    test_primality();
    test_pow();
    test_pollard_rho();
    test_exgcd();
    test_floor_sum();
    test_ex_floor_sum();
    test_frac();
    test_poly_mul();
    test_rmq();
    test_fenwick();
    test_segment_tree();
    test_max_flow();
    test_min_cost_flow();
    test_p2();
    test_p2r();
    test_convex_hull();
    test_kmp();
    test_z_function();
    test_manacher();
    test_lyndon();
    test_suffix_array();
}
