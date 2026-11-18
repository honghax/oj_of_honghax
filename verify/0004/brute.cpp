// 0004 独立暴力验证器：枚举事件子集
#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    const int RING = 28;
    int T;
    cin >> T;
    string out;
    while (T--) {
        int E, K;
        cin >> E >> K;
        vector<int> ts(E), vs(E);
        for (int i = 0; i < E; ++i) cin >> ts[i] >> vs[i];

        long long best = 0;
        for (int mask = 0; mask < (1 << E); ++mask) {
            int cnt = __builtin_popcount((unsigned)mask);
            if (cnt == 0) continue;
            bool ok = true;
            // 对每个被选事件作为窗口右端，检查同时发声数
            for (int i = 0; i < E && ok; ++i) {
                if (!(mask >> i & 1)) continue;
                int c = 0;
                for (int j = 0; j < E; ++j) {
                    if (!(mask >> j & 1)) continue;
                    if (ts[j] <= ts[i] && ts[j] + RING > ts[i]) ++c;
                }
                if (c > K) ok = false;
            }
            if (!ok) continue;
            long long sum = 0;
            for (int i = 0; i < E; ++i) if (mask >> i & 1) sum += vs[i];
            best = max(best, sum);
        }
        out += to_string(best);
        out.push_back('\n');
    }
    fputs(out.c_str(), stdout);
    return 0;
}
