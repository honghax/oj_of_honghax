// 0006 - 跟拍相机：最少校准段（二维走廊 + 可达区间 DP）
// 参考项目：E:\project\RedStoneMusic-local（redpiano_core.py cam_follow_plan 的采样/压缩）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;
typedef long long ll;
typedef __int128 i128;

struct Frac { ll num, den; };   // den > 0

static inline bool fracLess(const Frac &a, const Frac &b) {
    return (i128)a.num * b.den < (i128)b.num * a.den;
}

struct AxisCorridor {
    bool hasLo = false, hasHi = false;
    Frac lo{}, hi{};

    // 加入约束：slope ∈ [(dx-E)/dt, (dx+E)/dt]
    bool add(ll dx, ll E, ll dt) {
        Frac a{dx - E, dt}, b{dx + E, dt};
        if (!hasLo || fracLess(lo, a)) { lo = a; hasLo = true; }
        if (!hasHi || fracLess(b, hi)) { hi = b; hasHi = true; }
        return !(hasLo && hasHi && fracLess(hi, lo));
    }
};

int main() {
    int N;
    ll E;
    if (scanf("%d %lld", &N, &E) != 2) return 0;
    vector<ll> x(N + 1), z(N + 1);
    for (int i = 0; i <= N; ++i) scanf("%lld %lld", &x[i], &z[i]);

    // reach[s] = 从 s 出发（位置精确）时，最远能覆盖到的 tick
    vector<int> reach(N + 1, N);
    for (int s = 0; s < N; ++s) {
        AxisCorridor cx, cz;
        bool ok = true;
        for (int t = s + 1; t <= N; ++t) {
            const ll dt = t - s;
            if (!cx.add(x[t] - x[s], E, dt) || !cz.add(z[t] - z[s], E, dt)) {
                reach[s] = t - 1;
                ok = false;
                break;
            }
        }
        if (ok) reach[s] = N;
    }

    const int INF = 1e9;
    vector<int> dp(N + 1, INF);
    dp[0] = 0;
    for (int e = 1; e <= N; ++e) {
        for (int s = 0; s < e; ++s) {
            if (reach[s] >= e && dp[s] + 1 < dp[e]) dp[e] = dp[s] + 1;
        }
    }
    printf("%d\n", dp[N]);
    return 0;
}
