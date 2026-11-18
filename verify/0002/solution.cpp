// 0002 - 加权精确欧氏距离变换（最近岸格 + 平方距离）
// 参考项目：E:\project\1945\war3d（水系掩码 -> 到岸距离场 -> 深度着色）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll INF = (ll)4e18;
static const ll ZMIN = LLONG_MIN / 4;
static const ll ZMAX = LLONG_MAX / 4;

// 数学意义下的 floor(a / b)，b > 0
static inline ll floorDiv(ll a, ll b) {
    ll q = a / b, r = a % b;
    if (r != 0 && r < 0) --q;
    return q;
}

// f[q]：把中心放在 q 的代价；A：二次项系数
// out[x] = min_q f[q] + A*(x-q)^2，arg[x] 为取到最小值的最小 q
struct Transform1D {
    vector<int> v, arg;
    vector<ll> z, f, out;

    void run(const vector<ll> &cost, ll A) {
        const int n = (int)cost.size();
        f = cost;
        out.assign(n, INF);
        arg.assign(n, -1);
        v.assign(n, 0);
        z.assign(n + 1, 0);

        int k = -1;
        for (int q = 0; q < n; ++q) {
            if (f[q] >= INF) continue;
            ll fq = f[q] + A * (ll)q * q;
            if (k < 0) {
                k = 0;
                v[0] = q;
                z[0] = ZMIN;
                z[1] = ZMAX;
                continue;
            }
            bool placed = false;
            while (k >= 0) {
                const int p = v[k];
                ll fp = f[p] + A * (ll)p * p;
                ll num = fq - fp;
                ll den = 2LL * A * (q - p);
                ll s = floorDiv(num, den);   // 最后一个「p 不劣于 q」的整数 x
                if (s <= z[k]) {
                    --k;
                } else {
                    ++k;
                    v[k] = q;
                    z[k] = s;
                    z[k + 1] = ZMAX;
                    placed = true;
                    break;
                }
            }
            if (!placed) {
                k = 0;
                v[0] = q;
                z[0] = ZMIN;
                z[1] = ZMAX;
            }
        }
        if (k < 0) return;

        k = 0;
        for (int x = 0; x < n; ++x) {
            while (z[k + 1] < x) ++k;
            const int q = v[k];
            out[x] = f[q] + A * (ll)(x - q) * (x - q);
            arg[x] = q;
        }
    }
};

static inline void appendInt(string &s, ll v) {
    char buf[24];
    int n = 0;
    if (v == 0) buf[n++] = '0';
    while (v > 0) { buf[n++] = char('0' + (int)(v % 10)); v /= 10; }
    while (n > 0) s.push_back(buf[--n]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, DX, DY;
    if (!(cin >> W >> H >> DX >> DY)) return 0;

    vector<string> mask(H);
    for (int i = 0; i < H; ++i) cin >> mask[i];

    const ll A = (ll)DX * DX;
    const ll B = (ll)DY * DY;

    vector<ll> g((size_t)W * H, INF);
    vector<unsigned short> ac((size_t)W * H, 0);   // 行内最优列

    Transform1D tr;

    // 行变换：对每一行，求每个列位置到本行最近岸格的横向代价
    vector<ll> rowCost(W);
    for (int j = 0; j < H; ++j) {
        for (int x = 0; x < W; ++x) rowCost[x] = (mask[j][x] == '1') ? INF : 0;
        tr.run(rowCost, A);
        for (int x = 0; x < W; ++x) {
            const size_t id = (size_t)j * W + x;
            g[id] = tr.out[x];
            ac[id] = (unsigned short)(tr.arg[x] < 0 ? 0 : tr.arg[x]);
        }
    }

    // 转置，使列变换也变成连续访问（大图上显著更快）
    vector<ll> gt((size_t)H * W);
    vector<unsigned short> act((size_t)H * W);
    for (int j = 0; j < H; ++j)
        for (int x = 0; x < W; ++x) {
            const size_t src = (size_t)j * W + x;
            const size_t dst = (size_t)x * H + j;
            gt[dst] = g[src];
            act[dst] = ac[src];
        }
    g.clear(); g.shrink_to_fit();
    ac.clear(); ac.shrink_to_fit();

    // 列变换：对每一列，把行代价加上纵向二次项，得到全平面精确距离
    vector<unsigned short> art((size_t)H * W, 0);   // 转置布局：列 c 的第 j 行
    vector<ll> colCost(H);
    for (int x = 0; x < W; ++x) {
        const size_t base = (size_t)x * H;
        for (int j = 0; j < H; ++j) colCost[j] = gt[base + j];
        tr.run(colCost, B);
        for (int j = 0; j < H; ++j) {
            gt[base + j] = tr.out[j];
            art[base + j] = (unsigned short)(tr.arg[j] < 0 ? 0 : tr.arg[j]);
        }
    }

    int Q;
    cin >> Q;
    vector<int> qr(Q), qc(Q);
    for (int q = 0; q < Q; ++q) cin >> qr[q] >> qc[q];
    string out;
    out.reserve((size_t)Q * 32);
    for (int q = 0; q < Q; ++q) {
        int r = qr[q], c = qc[q];
        const size_t id = (size_t)c * H + r;
        if (gt[id] >= INF / 2) {
            out += "-1 -1 -1\n";
        } else {
            const int nr = (int)art[id];
            const int nc = (int)act[(size_t)c * H + nr];   // 最近行的行内最优列
            appendInt(out, nr);
            out.push_back(' ');
            appendInt(out, nc);
            out.push_back(' ');
            appendInt(out, gt[id]);
            out.push_back('\n');
        }
    }
    fputs(out.c_str(), stdout);
    return 0;
}
