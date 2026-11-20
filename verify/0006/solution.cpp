// 0006 - 湘北水文：最低出溢面（Priority-Flood / 桶队列）
// 参考项目：E:\project\1945\war3d（水系水位求解：从掩码与 DEM 解出水面的思路）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
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

    const int INF = 1e9;
    vector<int> lvl(N, INF);
    vector<vector<int>> bucket(maxh + 1);

    auto is_border = [&](int r, int c) { return r == 0 || r == H - 1 || c == 0 || c == W - 1; };
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c)
            if (is_border(r, c)) {
                int id = r * W + c;
                lvl[id] = a[id];
                bucket[a[id]].push_back(id);
            }

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    for (int l = 0; l <= maxh; ++l) {
        for (size_t p = 0; p < bucket[l].size(); ++p) {
            int u = bucket[l][p];
            if (lvl[u] != l) continue;
            int r = u / W, c = u % W;
            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
                int v = nr * W + nc;
                int nl = max(l, a[v]);
                if (nl < lvl[v]) {
                    lvl[v] = nl;
                    bucket[nl].push_back(v);
                }
            }
        }
    }

    int Q;
    scanf("%d", &Q);
    while (Q--) {
        int r, c;
        scanf("%d %d", &r, &c);
        printf("%d\n", lvl[r * W + c]);
    }
    return 0;
}
