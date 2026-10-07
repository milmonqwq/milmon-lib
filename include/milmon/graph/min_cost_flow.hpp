#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace cp {

template <class Cap, class Cost>
class MinCostFlow {
    static_assert(std::numeric_limits<Cap>::is_integer, "Cap must be an integer");
    static_assert(std::numeric_limits<Cost>::is_integer && std::numeric_limits<Cost>::is_signed,
                  "Cost must be a signed integer");

public:
    struct FlowEdge {
        int from, to;
        Cap cap, flow;
        Cost cost;
    };

    explicit MinCostFlow(int n = 0, int edge_hint = 0) : n(n), head(n, -1) {
        assert(n >= 0 && edge_hint >= 0);
        if (edge_hint > 0) {
            pos.reserve(edge_hint);
            residual.reserve(2ULL * edge_hint);
        }
    }

    inline int add_edge(int from, int to, Cap cap, Cost cost) {
        assert(0 <= from && from < n);
        assert(0 <= to && to < n);
        assert(Cap(0) <= cap);
        assert(cost != std::numeric_limits<Cost>::lowest());
        const int m = int(pos.size());
        const int id = int(residual.size());
        pos.emplace_back(from, id);
        residual.push_back(Edge{to, head[from], cap, cost});
        head[from] = id;
        residual.push_back(Edge{from, head[to], Cap(0), -cost});
        head[to] = id + 1;
        return m;
    }

    [[nodiscard]] inline FlowEdge get_edge(int i) const {
        assert(0 <= i && i < (int)pos.size());
        const Edge& e = residual[pos[i].second];
        const Edge& re = residual[pos[i].second ^ 1];
        return FlowEdge{pos[i].first, e.to, e.cap + re.cap, re.cap, e.cost};
    }

    [[nodiscard]] inline std::vector<FlowEdge> edges() const {
        std::vector<FlowEdge> result;
        result.reserve(pos.size());
        for (int i = 0; i < (int)pos.size(); ++i) result.push_back(get_edge(i));
        return result;
    }

    [[nodiscard]] inline std::pair<Cap, Cost> flow(int s, int t) {
        return flow(s, t, std::numeric_limits<Cap>::max());
    }

    inline std::pair<Cap, Cost> flow(int s, int t, Cap flow_limit) {
        assert(0 <= s && s < n);
        assert(0 <= t && t < n);
        assert(s != t);
        assert(Cap(0) <= flow_limit);
        Cap total_flow = Cap(0);
        Cost total_cost = Cost(0);
        if (flow_limit == Cap(0)) return {total_flow, total_cost};
        const Cost inf = std::numeric_limits<Cost>::max();
        std::vector<Cost> potential(n, Cost(0)), dist(n);
        std::vector<int> prev(n);
        bool has_negative = false;
        for (const Edge& e : residual) {
            if (e.cap > Cap(0) && e.cost < Cost(0)) { has_negative = true; break; }
        }
        if (has_negative) {
            std::fill(potential.begin(), potential.end(), inf);
            potential[s] = Cost(0);
            for (int k = 1; k < n; ++k) {
                bool changed = false;
                for (int v = 0; v < n; ++v) {
                    if (potential[v] == inf) continue;
                    for (int i = head[v]; i != -1; i = residual[i].next) {
                        const Edge& e = residual[i];
                        if (e.cap <= Cap(0) || potential[e.to] <= potential[v] + e.cost) continue;
                        potential[e.to] = potential[v] + e.cost;
                        changed = true;
                    }
                }
                if (!changed) break;
            }
            for (Cost& p : potential) if (p == inf) p = Cost(0);
        }
        using State = std::pair<Cost, int>;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
        while (total_flow < flow_limit) {
            std::fill(dist.begin(), dist.end(), inf);
            dist[s] = Cost(0);
            pq.emplace(Cost(0), s);
            while (!pq.empty()) {
                const auto [d, v] = pq.top();
                pq.pop();
                if (d != dist[v]) continue;
                for (int i = head[v]; i != -1; i = residual[i].next) {
                    const Edge& e = residual[i];
                    if (e.cap <= Cap(0)) continue;
                    const Cost nd = d + e.cost + potential[v] - potential[e.to];
                    if (nd >= dist[e.to]) continue;
                    dist[e.to] = nd;
                    prev[e.to] = i;
                    pq.emplace(nd, e.to);
                }
            }
            if (dist[t] == inf) break;
            const Cost unit_cost = dist[t] + potential[t] - potential[s];
            for (int v = 0; v < n; ++v) if (dist[v] != inf) potential[v] += dist[v];
            Cap pushed = flow_limit - total_flow;
            for (int v = t; v != s; v = residual[prev[v] ^ 1].to)
                pushed = std::min(pushed, residual[prev[v]].cap);
            for (int v = t; v != s; v = residual[prev[v] ^ 1].to) {
                residual[prev[v]].cap -= pushed;
                residual[prev[v] ^ 1].cap += pushed;
            }
            total_flow += pushed;
            total_cost += Cost(pushed) * unit_cost;
        }
        return {total_flow, total_cost};
    }

private:
    struct Edge {
        int to, next;
        Cap cap;
        Cost cost;
    };

    int n;
    std::vector<int> head;
    std::vector<std::pair<int, int>> pos;
    std::vector<Edge> residual;
};

}
