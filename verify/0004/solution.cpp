// 0004 - 并发发声上限下的最优事件取舍（加权区间选择 + 容量 K）
// 参考项目：E:\project\RedStoneMusic-local（07_build_scheduler.py 的 ring=28 / max_overlap / 247 上限）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

struct MinCostFlow {
    struct Edge { int to, next, cap, cost; };
    int n;
    vector<int> head;
    vector<Edge> e;

    explicit MinCostFlow(int n_) : n(n_), head(n_, -1) { e.reserve(1 << 20); }

    void addEdge(int u, int v, int cap, int cost) {
        e.push_back({v, head[u], cap, cost});
        head[u] = (int)e.size() - 1;
        e.push_back({u, head[v], 0, -cost});
        head[v] = (int)e.size() - 1;
    }

    // {最大流, 最小费用}，最多发送 limit 单位；Dijkstra + 势能
    pair<int, ll> run(int s, int t, int limit) {
        const ll INF = (ll)4e18;
        vector<ll> h(n, INF), dist(n);
        vector<int> preE(n), preV(n);

        // 初始图是 DAG（时间边 i->i+1，事件边 start->end），按编号顺序求初始势能
        h[s] = 0;
        for (int u = 0; u < n; ++u) {
            if (h[u] == INF) continue;
            for (int ei = head[u]; ei != -1; ei = e[ei].next) {
                if (e[ei].cap <= 0) continue;
                int v = e[ei].to;
                ll nd = h[u] + e[ei].cost;
                if (nd < h[v]) h[v] = nd;
            }
        }
        for (int i = 0; i < n; ++i) if (h[i] == INF) h[i] = 0;

        int flow = 0;
        ll cost = 0;
        while (flow < limit) {
            fill(dist.begin(), dist.end(), INF);
            fill(preE.begin(), preE.end(), -1);
            fill(preV.begin(), preV.end(), -1);

            priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<pair<ll, int>>> pq;
            dist[s] = 0;
            pq.push({0, s});
            while (!pq.empty()) {
                auto [d, u] = pq.top();
                pq.pop();
                if (d != dist[u]) continue;
                for (int ei = head[u]; ei != -1; ei = e[ei].next) {
                    if (e[ei].cap <= 0) continue;
                    int v = e[ei].to;
                    ll nd = d + e[ei].cost + h[u] - h[v];
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        preE[v] = ei;
                        preV[v] = u;
                        pq.push({nd, v});
                    }
                }
            }
            if (dist[t] == INF) break;

            for (int v = 0; v < n; ++v) if (dist[v] < INF) h[v] += dist[v];

            int aug = limit - flow;
            for (int v = t; v != s; v = preV[v]) aug = min(aug, e[preE[v]].cap);
            for (int v = t; v != s; v = preV[v]) {
                e[preE[v]].cap -= aug;
                e[preE[v] ^ 1].cap += aug;
            }
            flow += aug;
            cost += (ll)aug * (h[t] - h[s]);
        }
        return {flow, cost};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int RING = 28;   // 声音持续 28 刻（项目调度器常量）

    int T;
    cin >> T;
    string out;
    while (T--) {
        int E, K;
        cin >> E >> K;

        vector<int> ts(E), vs(E);
        vector<int> coord;
        coord.reserve(2 * E);
        for (int i = 0; i < E; ++i) {
            cin >> ts[i] >> vs[i];
            coord.push_back(ts[i]);
            coord.push_back(ts[i] + RING);
        }
        sort(coord.begin(), coord.end());
        coord.erase(unique(coord.begin(), coord.end()), coord.end());

        const int M = (int)coord.size();
        const int S = 0, TT = M - 1;

        MinCostFlow mcf(M);
        // 时间轴：相邻时刻之间容量 K，费用 0
        for (int i = 0; i + 1 < M; ++i) mcf.addEdge(i, i + 1, K, 0);
        // 每个事件：占用 [t, t+28)，选它收益 v
        for (int i = 0; i < E; ++i) {
            int a = (int)(lower_bound(coord.begin(), coord.end(), ts[i]) - coord.begin());
            int b = (int)(lower_bound(coord.begin(), coord.end(), ts[i] + RING) - coord.begin());
            mcf.addEdge(a, b, 1, -vs[i]);
        }

        auto res = mcf.run(S, TT, K);
        out += to_string(-res.second);
        out.push_back('\n');
    }
    fputs(out.c_str(), stdout);
    return 0;
}
