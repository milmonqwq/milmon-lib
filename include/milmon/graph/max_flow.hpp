#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

namespace cp {

template <class Cap>
class MaxFlow {
public:
    struct FlowEdge {
        int from, to;
        Cap cap, flow;
    };

    explicit MaxFlow(int n = 0, int edge_hint = 0) : n(n), head(n, -1) {
        assert(edge_hint >= 0);
        if (edge_hint > 0) {
            pos.reserve(edge_hint);
            residual.reserve(2ULL * edge_hint);
        }
    }

    inline int add_edge(int from, int to, Cap cap) {
        assert(0 <= from && from < n);
        assert(0 <= to && to < n);
        assert(Cap(0) <= cap);
        const int m = int(pos.size());
        const int id = int(residual.size());
        pos.emplace_back(from, id);
        residual.push_back(Edge{to, head[from], cap});
        head[from] = id;
        residual.push_back(Edge{from, head[to], Cap(0)});
        head[to] = id + 1;
        return m;
    }

    [[nodiscard]] inline FlowEdge get_edge(int i) const {
        assert(0 <= i && i < (int)pos.size());
        const Edge& e = residual[pos[i].second];
        const Edge& re = residual[pos[i].second ^ 1];
        return FlowEdge{pos[i].first, e.to, e.cap + re.cap, re.cap};
    }

    [[nodiscard]] inline std::vector<FlowEdge> edges() const {
        std::vector<FlowEdge> result;
        result.reserve(pos.size());
        for (int i = 0; i < (int)pos.size(); ++i) result.push_back(get_edge(i));
        return result;
    }

    inline void change_edge(int i, Cap new_cap, Cap new_flow) {
        assert(0 <= i && i < (int)pos.size());
        assert(Cap(0) <= new_flow && new_flow <= new_cap);
        Edge& e = residual[pos[i].second];
        Edge& re = residual[pos[i].second ^ 1];
        e.cap = new_cap - new_flow;
        re.cap = new_flow;
    }

    [[nodiscard]] inline Cap flow(int s, int t) {
        return flow(s, t, std::numeric_limits<Cap>::max());
    }

    inline Cap flow(int s, int t, Cap flow_limit) {
        assert(0 <= s && s < n);
        assert(0 <= t && t < n);
        assert(s != t);
        assert(Cap(0) <= flow_limit);
        const Cap zero = Cap(0);
        struct PathNode { int v, edge; Cap cap; };
        std::vector<int> level(n), iter, que(n);
        std::vector<PathNode> path;
        path.reserve(n);
        auto bfs = [&]() {
            std::fill(level.begin(), level.end(), -1);
            level[s] = 0;
            int qh = 0, qt = 0;
            que[qt++] = s;
            while (qh < qt) {
                const int v = que[qh++];
                for (int i = head[v]; i != -1; i = residual[i].next) {
                    const Edge& e = residual[i];
                    if (e.cap == zero || level[e.to] != -1) continue;
                    level[e.to] = level[v] + 1;
                    if (e.to == t) return;
                    que[qt++] = e.to;
                }
            }
        };
        Cap flow = Cap(0);
        while (flow < flow_limit) {
            bfs();
            if (level[t] < 0) break;
            iter = head;
            path.clear();
            path.push_back(PathNode{s, -1, flow_limit - flow});
            while (!path.empty()) {
                const int v = path.back().v;
                if (v == t) {
                    const Cap d = path.back().cap;
                    int cut = -1;
                    for (int i = 1; i < (int)path.size(); ++i) {
                        Edge& e = residual[path[i].edge];
                        e.cap -= d;
                        residual[path[i].edge ^ 1].cap += d;
                        if (cut == -1 && e.cap == zero) cut = i;
                    }
                    flow += d;
                    if (!(flow < flow_limit)) break;
                    assert(cut != -1);
                    for (int i = 0; i < cut; ++i) path[i].cap -= d;
                    iter[path[cut - 1].v] = residual[path[cut].edge].next;
                    path.resize(cut);
                    continue;
                }
                int& i = iter[v];
                while (i != -1 && (residual[i].cap == zero ||
                                    level[residual[i].to] != level[v] + 1)) {
                    i = residual[i].next;
                }
                if (i == -1) {
                    level[v] = -1;
                    path.pop_back();
                    continue;
                }
                const Edge& e = residual[i];
                path.push_back(PathNode{e.to, i, std::min(path.back().cap, e.cap)});
            }
        }
        return flow;
    }

    [[nodiscard]] inline std::vector<bool> min_cut(int s) const {
        assert(0 <= s && s < n);
        std::vector<bool> visited(n);
        std::vector<int> que;
        que.reserve(n);
        que.push_back(s);
        visited[s] = true;
        for (int qh = 0; qh < (int)que.size(); ++qh) {
            const int v = que[qh];
            for (int i = head[v]; i != -1; i = residual[i].next) {
                const Edge& e = residual[i];
                if (e.cap == Cap(0) || visited[e.to]) continue;
                visited[e.to] = true;
                que.push_back(e.to);
            }
        }
        return visited;
    }

private:
    struct Edge {
        int to, next;
        Cap cap;
    };

    int n;
    std::vector<int> head;
    std::vector<std::pair<int, int>> pos;
    std::vector<Edge> residual;
};

}
