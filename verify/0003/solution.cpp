// 0003 - 屏幕空间标签避让（精确模拟 + 二维矩形加/矩形和）
// 参考项目：E:\project\1945\war3d（main.cpp DrawLabels 的“重叠则上移，三次仍冲突则丢弃”）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

// 二维树状数组：矩形加 + 矩形和（四个 BIT 的经典写法）
struct BIT2D {
    int nx, ny;
    vector<ll> b1, b2, b3, b4;

    BIT2D(int nx_, int ny_) : nx(nx_), ny(ny_) {
        const size_t n = (size_t)(nx + 2) * (ny + 2);
        b1.assign(n, 0);
        b2.assign(n, 0);
        b3.assign(n, 0);
        b4.assign(n, 0);
    }

    inline void addPoint(int x, int y, ll v) {
        const ll vx = v * (x - 1);
        const ll vy = v * (y - 1);
        const ll vxy = vx * (y - 1);
        for (int i = x; i <= nx; i += i & -i) {
            const size_t row = (size_t)i * (ny + 2);
            for (int j = y; j <= ny; j += j & -j) {
                const size_t id = row + j;
                b1[id] += v;
                b2[id] += vx;
                b3[id] += vy;
                b4[id] += vxy;
            }
        }
    }

    inline void addRect(int x1, int x2, int y1, int y2, ll v) {
        addPoint(x1, y1, v);
        addPoint(x1, y2 + 1, -v);
        addPoint(x2 + 1, y1, -v);
        addPoint(x2 + 1, y2 + 1, v);
    }

    inline ll prefix(int x, int y) const {
        ll s1 = 0, s2 = 0, s3 = 0, s4 = 0;
        for (int i = x; i > 0; i -= i & -i) {
            const size_t row = (size_t)i * (ny + 2);
            for (int j = y; j > 0; j -= j & -j) {
                const size_t id = row + j;
                s1 += b1[id];
                s2 += b2[id];
                s3 += b3[id];
                s4 += b4[id];
            }
        }
        return s1 * x * y - s2 * y - s3 * x + s4;
    }

    inline ll rectSum(int x1, int x2, int y1, int y2) const {
        return prefix(x2, y2) - prefix(x1 - 1, y2) - prefix(x2, y1 - 1)
               + prefix(x1 - 1, y1 - 1);
    }
};

static inline void appendInt(string &s, ll v) {
    char buf[24];
    int n = 0;
    if (v < 0) { s.push_back('-'); v = -v; }
    if (v == 0) buf[n++] = '0';
    while (v > 0) { buf[n++] = char('0' + (int)(v % 10)); v /= 10; }
    while (n > 0) s.push_back(buf[--n]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, N;
    if (!(cin >> W >> H >> N)) return 0;

    // 标签最多上移 2 次，每次 h+5；h <= 200 -> 上移不超过 410
    const int YOFF = 620;
    const int NX = W + 2;
    const int NY = H + YOFF + 2;
    BIT2D bit(NX, NY);

    string out;
    out.reserve((size_t)N * 12);

    for (int i = 0; i < N; ++i) {
        int x, y, w, h;
        cin >> x >> y >> w >> h;

        // 项目原逻辑：最多尝试 3 个位置（初始、上移一次、上移两次）
        bool placed = false;
        int fy = y;
        for (int attempt = 0; attempt < 3; ++attempt) {
            const int yy = y - attempt * (h + 5);
            const int x1 = x + 1;
            const int x2 = x + w;              // 半开区间 [x, x+w)
            const int y1 = yy + YOFF + 1;
            const int y2 = yy + h + YOFF;      // [yy, yy+h)
            // 严格相交：矩形内部有正面积重叠
            if (bit.rectSum(x1, x2, y1, y2) == 0) {
                bit.addRect(x1, x2, y1, y2, 1);
                fy = yy;
                placed = true;
                break;
            }
        }

        if (placed) {
            out += "1 ";
            appendInt(out, x);
            out.push_back(' ');
            appendInt(out, fy);
            out.push_back('\n');
        } else {
            out += "0\n";
        }
    }

    fputs(out.c_str(), stdout);
    return 0;
}
