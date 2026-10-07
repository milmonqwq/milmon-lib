# milmon-lib

一个可扩展的 C++17 算法竞赛模板库。开发时只需要引入统一入口：

```cpp
#include <milmon/all.hpp>
```

提交前运行 Python 展开器，它会根据源码中真正出现的标识符选择模块，递归补齐依赖，并把本地 `include` 替换成自包含的 C++ 代码。

环境要求为 GCC/Clang、C++17 和 Python 3.9 或更新版本，展开器本身没有第三方依赖。

## 快速开始

开发时给编译器增加 `include` 搜索路径：

```bash
g++ -std=c++17 -O2 -Iinclude examples/basic.cpp -o basic
```

生成提交文件：

```bash
python3 tools/bundle.py examples/basic.cpp -o submission.cpp --explain
g++ -std=c++17 -O2 submission.cpp -o submission
```

`--explain` 会在标准错误中显示每个模块为什么被选中，不会污染生成的源码。也可以把结果输出到标准输出：

```bash
python3 tools/bundle.py solution.cpp > submission.cpp
```

## 已有模块

目录按用途组织：

```text
include/milmon/
├── types.hpp
├── basic.hpp
├── debug.hpp
├── ds/
│   ├── dsu.hpp
│   ├── ddsu.hpp
│   ├── fenwick.hpp
│   ├── rmq.hpp
│   └── segment_tree.hpp
├── geometry/
│   ├── convex_hull.hpp
│   ├── p2.hpp
│   └── p2r.hpp
├── graph/
│   ├── max_flow.hpp
│   └── min_cost_flow.hpp
├── math/
│   ├── exgcd.hpp
│   ├── ex_floor_sum.hpp
│   ├── floor_sum.hpp
│   ├── frac.hpp
│   ├── poly/
│   │   └── poly_mul.hpp
│   ├── pow.hpp
│   ├── primality.hpp
│   └── pollard_rho.hpp
├── misc/
│   ├── random.hpp
│   └── splitmix64.hpp
└── string/
    ├── kmp.hpp
    ├── lyndon.hpp
    ├── manacher.hpp
    ├── suffix_array.hpp
    └── z_function.hpp
```

### 全局类型别名

引入统一入口后，可以直接使用下列名称，无需添加 `cp::`：

```cpp
u32 unsigned_32;
u64 unsigned_64;
i32 signed_32;
i64 signed_64;
pii integer_pair;
ll signed_64_short;
ull unsigned_64_short;
ld real_extended;
u128 unsigned_128;
i128 signed_128;
```

其中 `u32/u64` 分别对应 `std::uint32_t/std::uint64_t`，`i32/i64` 分别对应 `int/long long`，`pii` 对应 `std::pair<int, int>`；`ll/ull` 分别是 `long long/u64` 的简写，`ld` 是 `long double` 的简写。展开器会识别这些别名，自动引入 `types` 模块。

### SplitMix64

```cpp
u64 mixed = cp::splitmix64(value);
```

`misc/splitmix64.hpp` 提供无状态的 64 位整数混合函数：相同输入始终返回相同结果，不修改输入，支持 `constexpr` 求值。运算按模 `2^64` 自然回绕，可用于整数哈希；Pollard–Rho 的内部随机序列也复用此混合函数。

### 随机数

```cpp
cp::rng.seed(12345);                 // 可选：显式指定种子
auto raw = cp::rng();                // mt19937_64 原始输出
int value = cp::rand(-5, 5);         // 闭区间 [-5, 5]
i64 large = cp::rand<i64>(0, 1000000000000LL);
```

`misc/random.hpp` 提供 `inline std::mt19937_64 rng`，默认使用 `std::chrono::steady_clock` 的当前计数作为种子；多个翻译单元共享同一个引擎。`rand<T>(l, r)` 使用 `std::uniform_int_distribution<T>` 在闭区间 `[l, r]` 上均匀生成整数，支持标准库接受的有符号与无符号整数类型，要求 `l <= r`；省略模板参数时两端须为同一类型。引擎不是密码学随机源，并发访问需要调用方同步。

### 基础 I/O 设置

```cpp
cp::init_io();
std::cout << value << endl;
```

`basic.hpp` 将 `endl` 定义为 `'\n'`。`init_io(precision)` 会关闭 iostream 与 stdio 同步，解除 `cin`、`cout` 的绑定，并将 `cout`、`cerr` 的浮点数输出设置为 `fixed` 和指定精度；`precision` 默认为 `10`。

需要使用文件输入输出时，可以调用：

```cpp
cp::file_io("example"); // example.in -> stdin，stdout -> example.out
```

### Debug 输出

```cpp
std::vector<int> values{1, 2, 3};
debug("value = %d\n", value);
// stderr: value = 42
dbg(value, values);
// stderr: value=42, values=[1,2,3]
```

`debug(...)` 保持为 `std::fprintf(stderr, __VA_ARGS__)`。`dbg(...)` 会将每个表达式的文本和值输出到 `std::cerr`，并支持直接输出 `std::vector` 等可迭代容器；`std::string` 仍按字符串输出。

### DSU

```cpp
cp::DSU dsu(n);
dsu.unite(a, b);
bool connected = dsu.same(a, b);
int component_size = dsu.size(a);
```

实现只保存一个 `val` 数组：负数表示根，其绝对值为连通块大小；非负数表示父节点编号。合并采用按大小合并，并使用路径压缩，不额外维护连通块数量。

### DDSU

```cpp
cp::DDSU dsu(n);
bool merged = dsu.unite(a, b);
int root = dsu.find(a);
bool connected = dsu.same(a, b);
dsu.reset(n);
```

头文件为 `<milmon/ds/ddsu.hpp>`。只保存父节点数组，初始时每个节点的父亲是自身，根满足 `parent[x] == x`，不使用 `-1` 标记。`unite(a, b)` 固定将 `b` 的根挂到 `a` 的根，不按大小合并；已经连通时返回 `false`。`find` 使用迭代式路径压缩，将查询路径上的节点直接挂到根，不使用递归。支持默认构造和 `reset(n)`，不维护连通块大小，也不提供 `size`。

### 快速素数判定

```cpp
bool prime = cp::is_prime(value);
```

实现是确定性的 64 位 Miller–Rabin，支持所有不超过 64 位的有符号或无符号整数；负数返回 `false`。乘法取模使用竞赛环境中常见的 GCC/Clang `unsigned __int128`。

### 扩展欧几里得算法

```cpp
long long x, y;
long long g = cp::exgcd(a, b, x, y);
```

返回非负的 `gcd(a, b)`，并通过 `x`、`y` 给出一组满足 `a*x+b*y=g` 的贝祖系数。参数类型需一致且为有符号整数，支持 `int` 和 `long long`；`a=b=0` 时返回 `0`。

### 整除分块求和

```cpp
i64 sum = cp::floor_sum(n, m, a, b);
```

计算 `0 <= i < n` 时 `floor((a*i+b)/m)` 的总和，时间复杂度为 O(log m)。要求 `n >= 0`、`m > 0` 且答案能用 `i64` 表示；`a`、`b` 可以为负数。

`ex_floor_sum(n, m, a, b)` 使用相同的参数约定，返回 `{sum(f_i), sum(i*f_i), sum(f_i*f_i)}` 组成的 `std::tuple<i128, i128, i128>`，其中 `f_i=floor((a*i+b)/m)`；要求三个结果都能用 `i128` 表示。

### 分数

```cpp
cp::frac<i128> a{1, 2}, b{1, 3};
auto sum = a + b; // 5/6
```

`frac<T>` 要求 `T` 为有符号整数类型，可以直接使用 `i128`。成员 `num`、`den` 分别表示分子和分母；构造后会自动约分并保证分母为正。支持四则运算、复合赋值、与整数混合运算和全部比较运算；`value()` 返回 `long double` 近似值。要求分母非零、除数非零，用户需要自行保证四则运算不溢出；对 `frac<i128>` 进行比较时，还需保证交叉相乘不会超出 `i128` 的表示范围。

### 模幂与模逆

```cpp
u32 value = cp::pow<1000000007>(base, exponent);
u32 inverse = cp::inv<1000000007>(value);
```

`pow<P>(x, y)` 计算 `x^y mod P`，`inv<P>(x)` 使用费马小定理计算乘法逆元。要求 `P` 为质数且 `x` 不被 `P` 整除。

### 多项式乘法

```cpp
cp::poly::init();
std::vector<u32> product = cp::poly::poly_mul(a, b);
```

系数按模数 `998244353` 计算，返回数组的长度为 `a.size()+b.size()-1`；任一输入为空时返回空数组。实现使用 NTT，时间复杂度为 O(n log n)，支持的变换长度最大为 `2^23`。`init()` 会预计算各级单位根，可以重复调用；`poly_mul()` 也会自动完成尚未进行的初始化。

### Pollard–Rho 分解

```cpp
u64 divisor = cp::pollard_rho(n); // n 为合数时返回一个非平凡因子
std::vector<u64> factors = cp::factorize(n); // 有序质因子，包含重数
std::vector<std::pair<u64, int>> powers = cp::factorize_pair(n); // {质因子, 指数}
```

`pollard_rho` 对质数返回其自身。`factorize` 与 `factorize_pair` 对小于 2 的数返回空数组；后者的每项 `{p, c}` 表示 `p^c`。其余情况结合确定性 Miller–Rabin 递归分解。实现采用 Brent 批量 GCD 版本的 Pollard–Rho。

### RMQ

```cpp
std::vector<int> values = {5, 2, 7, 1, 3};

cp::RMQ<int> minimum(values);
int answer = minimum.query(1, 4);                // min(values[1..4)) = 1
std::size_t position = minimum.query_index(1, 4); // 位置 3

cp::RMQ<int, std::greater<int>> maximum(values, std::greater<int>{});
int largest = maximum.query(1, 4);               // 7
```

区间统一为左闭右开 `[left, right)`，并要求 `left < right <= size()`。默认比较器为 `std::less<T>`；自定义比较器满足“第一个参数优于第二个参数时返回 `true`”即可。相等元素返回区间内最左下标。

实现使用 64 位块内单调栈掩码和块间稀疏表。在 64 位 word-RAM 模型下初始化为 O(n)，查询严格 O(1)，块内查询不调用比较器，跨块查询最多比较三个候选值。

### 树状数组

```cpp
cp::Fenwick<i64> sums(n);
sums.add(index, delta);             // values[index] += delta
i64 prefix = sums.prefix_sum(right); // sum(values[0..right))
```

下标为 0-based，前缀区间为左闭右开 `[0, right)`。也可以从数组线性初始化：

```cpp
cp::Fenwick<i64> sums(values);
```

### 线段树

```cpp
cp::SegmentTree<int> minimum({5, 2, 7, 1, 3});
minimum.set(3, 8);                           // values[3] = 8
minimum.add(1, 4);                           // values[1] += 4
int answer = minimum.query(0, 4);            // 区间最小值 5
std::size_t position = minimum.query_index(0, 4); // 位置 0
int smallest = minimum.query();             // 全局最小值 3
std::size_t smallest_position = minimum.query_index(); // 位置 4
```

与树状数组一样，支持默认空构造、按长度构造 `cp::SegmentTree<T>(n)`（元素初始化为 `T{}`）、从 `std::vector<T>` 或初始化列表构造，以及 `size()`、`empty()`。也支持 `cp::SegmentTree<T>(n, value)`，将所有位置初始化为同一个 `T` 值，例如 `cp::SegmentTree<int> filled(5, 7)` 创建五个值为 7 的元素。`set(i, value)` 单点赋值，`add(i, delta)` 单点累加；修改要求 `i < size()`，使用 `add` 时要求 `T` 支持 `+=`。

查询接口与 RMQ 一致：下标为 0-based，区间为左闭右开 `[left, right)`，要求 `left < right <= size()`；无参查询要求非空。`query` 返回元素的 `const T&`，`query_index` 返回其原数组下标。

底层数组 `std::vector<T> a` 为 public，可通过 `tree.a[i]` 访问当前元素。应通过 `set`/`add` 修改元素，并保持 `a` 的长度不变；直接写入 `a` 不会自动更新树节点。

模板参数为 `SegmentTree<T, Compare = std::less<T>>`，各构造函数均支持将比较器作为最后一个参数传入：`(n, comp)`、`(n, value, comp)` 或 `(values, comp)`。默认求最小值；比较器须满足严格弱序，“第一个参数优于第二个参数时返回 `true`”。比较器判定等价的元素返回最左下标，不要求元素支持 `==` 或数值哨兵。例如按记录的第一项求最小值：

```cpp
auto cmp = [](const auto& l, const auto& r) { return l.first < r.first; };
cp::SegmentTree records(std::vector<std::pair<int, int>>{{3, 10}, {1, 20}}, cmp);
auto record = records.query(); // {1, 20}
cp::SegmentTree repeated(5, std::pair<int, int>{3, 10}, cmp); // 五条相同记录
```

实现采用紧凑的 `2n` 非递归线段树，不补齐到二次幂；节点仅保存最优元素的下标。初始化 O(n)，单点修改和区间查询 O(log n)，全局查询 O(1)，空间 O(n)。线段树本身在修改和查询时不分配内存，查询不复制元素。

### 最大流

```cpp
cp::MaxFlow<i64> mf(n);
int id = mf.add_edge(from, to, cap); // 0-based，返回边的编号
i64 max_flow = mf.flow(source, sink);
i64 partial = mf.flow(source, sink, limit); // 最多只推 limit 流量
std::vector<bool> side = mf.min_cut(source); // 残量网络上从 source 可达的点
```

`MaxFlow<Cap>(n, edge_hint)` 的第二个参数可选，省略时为 0；它会预留原始边与两倍残量边空间，已知边数时可减少建图期间的扩容。`Cap` 支持 `int`、`i64` 等整数，也支持 `double` 等浮点数。实现为 Dinic 算法，使用手工栈替代递归，时间复杂度为 O(V²E)；`flow(s, t)` 等价于 `flow(s, t, std::numeric_limits<Cap>::max())`，可以反复调用以增量推送流量。

如需读取或修改每条边的状态：

```cpp
auto [from, to, cap, flow] = mf.get_edge(id);
std::vector<cp::MaxFlow<i64>::FlowEdge> all = mf.edges();
mf.change_edge(id, new_cap, new_flow); // 重置为「容量 new_cap、当前流量 new_flow」
```

`get_edge` 返回的 `cap` 为原始容量，`flow` 为当前流量；`change_edge` 可直接修改反向边上的流量。`add_edge`、`change_edge`、`get_edge` 与 `flow` 均带参数合法性断言。`min_cut` 返回的 `visited[v]` 为 `true` 当且仅当残量网络上存在 `source` 到 `v` 的边，可与最大流一起构成最小割。

### 最小费用流

```cpp
cp::MinCostFlow<i64, i64> mcf(n, edge_hint);
int id = mcf.add_edge(from, to, cap, cost); // 返回 0-based 边编号
auto [flow, cost] = mcf.flow(source, sink, limit);
auto [remaining, extra_cost] = mcf.flow(source, sink); // 继续推送剩余流量
auto [u, v, capacity, used, unit_cost] = mcf.get_edge(id);
auto all = mcf.edges();
```

`Cap` 与 `Cost` 分别为整数容量与有符号整数单位费用；支持负费用边。`flow` 返回本次调用实际推送的流量及其增量费用，无法达到 `limit` 时只返回可推送部分；省略 `limit` 时尽可能多推送。`get_edge` 的 `cap` 是原始容量，`flow` 是当前流量。求流时要求从源点可达的残量网络没有负费用环，且费用计算（包括中间距离、势能和总费用）与容量转换均在 `Cost` 范围内。实现采用首次 Bellman–Ford 初始化势能（仅有负残量边时）和逐次最短路 Dijkstra；重复调用会重新计算势能，以处理反向边上的负费用。

### 二维几何

```cpp
cp::p2 a{1, 2}, b{3, 4};
cp::p2 sum = a + b;
long long inner = cp::dot(a, b);
long long area2 = cp::cross(a, b);

cp::p2r<double> u{1.5, 2.0}, v{3.0, 4.5};
cp::p2r<double> midpoint = (u + v) / 2;
double real_area2 = cp::cross(u, v);

std::vector<cp::p2> hull = cp::convex_hull(points);
```

`p2` 包含两个 `int` 成员 `x`、`y`；`p2r<T>` 是对应的实数模板，`T` 可以使用 `double` 或 `long double`。两者都支持比较、向量加减、取负、数乘以及对应的复合赋值，`p2r<T>` 还支持除法。`dot(a, b)` 和 `cross(a, b)` 分别计算点积和叉积；`cross(o, a, b)` 计算向量 `a-o` 与 `b-o` 的叉积。

`convex_hull` 使用单调链算法，时间复杂度为 O(n log n)。输入点可以无序或重复；返回结果从字典序最小点开始逆时针排列，不重复首点，并移除凸包边上的共线中间点。少于两个不同点时直接返回去重后的结果。

### KMP

```cpp
std::vector<int> prefix = cp::kmp(sequence);
std::vector<int> matches = cp::kmp(text, pattern);
```

单参数的 `kmp` 与 `prefix_function` 等价，返回前缀函数；双参数版本返回 `pattern` 在 `text` 中的所有匹配起点，包含重叠匹配。空模式会匹配 `0..text.size()` 的所有位置。

参数可以是 `std::string` 或 `std::vector<int>`，时间复杂度为 O(n)。

### Z 函数

```cpp
std::vector<int> z = cp::z_function(sequence);
```

`z[i]` 表示从 `i` 开始的后缀与整个序列的最长公共前缀长度，约定 `z[0] = 0`。支持 `std::string` 和 `std::vector<int>`，时间复杂度为 O(n)。

### Manacher

```cpp
std::vector<int> radius = cp::manacher(sequence);
```

返回数组长度为 `2*n+1`。`radius[2*i]` 表示以第 `i-1`、`i` 个元素之间为中心的最长偶回文长度，`radius[2*i+1]` 表示以第 `i` 个元素为中心的最长奇回文长度。首尾间隙分别是下标 `0` 和 `2*n`，构建时间为 O(n)。

### Lyndon 分解

```cpp
std::vector<std::pair<int, int>> factors = cp::build_lyndon(sequence);
```

返回 Duval 分解得到的 Lyndon 因子区间，每个区间均为左闭右开 `[left, right)`，总时间复杂度为 O(n)。参数可以是 `std::string` 或 `std::vector<int>`。

### 后缀数组

```cpp
std::string text = "banana";
cp::SA suffixes(text);

int position = suffixes.sa[0]; // 第 0 名后缀从位置 5 开始
int rank = suffixes.rnk[0];    // 从位置 0 开始的后缀排名为 3
int common = suffixes.lcp(1, 3); // "anana" 与 "ana" 的 LCP 为 3

for (auto [left, right, period] : suffixes.runs()) {
    // [left, right) 是极大周期区间，period 是最小周期
}
```

`cp::SuffixArray` 与简写 `cp::SA` 等价，只接受 `std::string` 或 `std::vector<int>`：

```cpp
std::vector<int> values = {10, -3, 10, 7};
cp::SuffixArray suffixes(values);
```

只需要单独的 SA 或排名数组时，可以跳过 LCP 和 RMQ 构建：

```cpp
std::vector<int> sa = cp::build_sa(text);
std::vector<int> rnk = cp::build_rnk(values);
```

整数数组使用定长基数排序离散化，随后使用 SA-IS，构建时间为线性。若数组已经位于 `[0, upper]`，可以跳过离散化：

```cpp
cp::SuffixArray suffixes(values, upper);
std::vector<int> sa = cp::build_sa(values, upper);
std::vector<int> rnk = cp::build_rnk(values, upper);
```

`sa[rank]` 表示该排名的后缀起点，`rnk[position]` 表示该位置后缀的排名。`lcp(i, j)` 查询从位置 `i`、`j` 开始的最长公共前缀，预处理线性、单次查询 O(1)。

`runs()` 返回按 `(left, right, period)` 排序的 `std::vector<std::array<int, 3>>`。区间使用左闭右开形式，周期为最小周期；每次调用使用正反字典序 Lyndon root、正反向 LCP 和线性基数去重，在 O(n) 时间与空间内枚举全部 runs。

## 模块选择规则

展开器读取 [library.json](library.json)，扫描代码标识符并匹配每个模块的 `symbols`。扫描时会忽略注释、字符/字符串字面量和原始字符串，因此注释里的 `DSU` 不会误引入模块。

生成提交文件时，源码与模板模块需要的标准库 `#include <...>` 会去重并集中到文件开头。每个展开模块前会写入 `// milmon-lib/路径.hpp`，最后写入 `// milmon-lib ends`；没有选中任何模板模块时不会生成这些标记。

若源码没有实际包含任何 milmon-lib 头文件，展开器会原样输出源码，不修改 include、空行或其他内容。

静态扫描无法覆盖通过宏拼接等方式隐藏名字的情况。此时可以在源码中声明依赖：

```cpp
// milmon: require dsu, primality
```

或者在命令行指定：

```bash
python3 tools/bundle.py solution.cpp --require dsu --require fenwick -o submission.cpp
```

查看模块名和触发符号：

```bash
python3 tools/bundle.py --list
```

也支持直接引入 `#include <milmon/ds/dsu.hpp>`、`#include <milmon/string/kmp.hpp>` 或 `#include <milmon/math/pollard_rho.hpp>`；直接引入的模块始终会被展开。

## 增加算法模块

以新增 `segment_tree` 为例：

1. 创建 `include/milmon/ds/segment_tree.hpp`，使用 `#pragma once`，并让代码在 `namespace cp` 中保持可独立编译。
2. 在 `include/milmon/all.hpp` 中引入它。
3. 在 `library.json` 的 `modules` 中登记头文件、触发标识符和依赖：

```json
"segment_tree": {
  "header": "include/milmon/ds/segment_tree.hpp",
  "symbols": ["SegmentTree"],
  "depends": []
}
```

如果模块依赖其他模块，把模块名写进 `depends`。展开器会进行拓扑展开，并从模块源码中移除库内头文件的 `include`，避免重复定义。`symbols` 应选择用户代码会直接写出的、尽量独特的公开名称。

## 测试

```bash
make test
```

测试会检查所有模块，并验证模块依赖、目录内直接引入，以及生成的提交源码在没有 `-Iinclude` 的情况下仍可独立编译和运行。

每次 push 时，GitHub Actions 会运行 `make test`。
