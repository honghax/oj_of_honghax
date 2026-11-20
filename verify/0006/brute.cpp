// 0006 独立暴力验证器：二分 + BFS（小图）
#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    int W, H;
    if (scanf("%d %d", &W, &H) != 2) return 0;
    const int N = W * H;
    vector<int> a(N);
    int maxh = 0;
    for (int i = 0; i < N; ++i) { scanf("%d", &a[i]); maxh = max(maxh, a[i]); }

    auto border = [&](int r, int c) { return r == 0 || r == H - 1 || c == 0 || c == W - 1; };
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    int Q; scanf("%d", &Q);
    while (Q--) {
        int qr, qc; scanf("%d %d", &qr, &qc);
        auto can = [&](int L) {
            if (a[qr * W + qc] > L) return false;
            vector<char> vis(N, 0);
            vector<int> st;
            st.push_back(qr * W + qc); vis[qr * W + qc] = 1;
            while (!st.empty()) {
                int u = st.back(); st.pop_back();
                int r = u / W, c = u % W;
                if (border(r, c)) return true;
                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
                    int v = nr * W + nc;
                    if (!vis[v] && a[v] <= L) { vis[v] = 1; st.push_back(v); }
                }
            }
            return false;
        };
        int lo = a[qr * W + qc], hi = maxh;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (can(mid)) hi = mid; else lo = mid + 1;
        }
        printf("%d\n", lo);
    }
    return 0;
}
