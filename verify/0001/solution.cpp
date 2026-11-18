// 0001 - 2x2x2 魔方 HTM 最短复原（距离 + 方案数 + 字典序最小解）
// 参考项目：E:\project\CuBe\vcube（最优 HTM 求解器 / 剪枝表思路）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int NS  = 3674160;   // 7! * 3^6

// ---------------- 基础向量 / 旋转 ----------------
struct V3 { int x, y, z; };
static inline V3 operator+(V3 a, V3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline V3 operator-(V3 a)       { return {-a.x, -a.y, -a.z}; }
static inline V3 operator*(int k, V3 a){ return {k * a.x, k * a.y, k * a.z}; }
static inline bool operator==(V3 a, V3 b){ return a.x == b.x && a.y == b.y && a.z == b.z; }

static const V3 EX{1, 0, 0}, EY{0, 1, 0}, EZ{0, 0, 1};

struct Rot { V3 m[3]; };                       // 基向量的像
static inline V3 applyR(const Rot &r, V3 v) {
    return (v.x * r.m[0]) + (v.y * r.m[1]) + (v.z * r.m[2]);
}
static inline Rot comp(const Rot &A, const Rot &B) {   // A∘B：先 B 后 A
    Rot C;
    for (int i = 0; i < 3; ++i) C.m[i] = applyR(A, B.m[i]);
    return C;
}

// ---------------- 8 个角块位置 ----------------
// 0 URF  1 UFL  2 ULB  3 UBR  4 DFR  5 DLF  6 DBL  7 DRB
static const int PX[8] = { 1, -1, -1,  1,  1, -1, -1,  1};
static const int PY[8] = { 1,  1,  1,  1, -1, -1, -1, -1};
static const int PZ[8] = { 1,  1, -1, -1,  1,  1, -1, -1};

// 6 个面法向：0:+x(R) 1:-x(L) 2:+y(U) 3:-y(D) 4:+z(F) 5:-z(B)
static const V3 NV[6] = { {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };

static int pn[8][3];                 // 每个位置的三条外法向（已按右手序排列）

// 每个面顺时针（从面外看）90°的旋转：U R F D L B
static Rot base[6];
static int faceAxis[6] = {1, 0, 2, 1, 0, 2};
static int faceSign[6] = {1, 1, 1, -1, -1, -1};
static int layerMask[6];

static vector<Rot> rots;             // 24 个整体旋转
static int pmap[24][8];              // 位置映射
static int nmap[24][6];              // 法向映射
static int smap[24][8][3];           // 槽位映射

static inline int rotIndex(const Rot &r) {
    for (int i = 0; i < (int)rots.size(); ++i) {
        bool ok = true;
        for (int k = 0; k < 3 && ok; ++k)
            if (!(rots[i].m[k] == r.m[k])) ok = false;
        if (ok) return i;
    }
    return -1;
}

static int parity3(const int p[3]) {
    int inv = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = i + 1; j < 3; ++j)
            if (p[i] > p[j]) ++inv;
    return (inv & 1) ? -1 : 1;
}

static void buildGeometry() {
    // 位置法向三元组；若手性为负则交换后两个，保证三组都右手
    for (int i = 0; i < 8; ++i) {
        int xid = PX[i] > 0 ? 0 : 1;
        int yid = PY[i] > 0 ? 2 : 3;
        int zid = PZ[i] > 0 ? 4 : 5;
        if (PX[i] * PY[i] * PZ[i] < 0) swap(yid, zid);
        pn[i][0] = xid; pn[i][1] = yid; pn[i][2] = zid;
    }
    // 24 个真旋转（行列式为 +1 的带符号置换）
    int p[3] = {0, 1, 2};
    V3 e[3] = {EX, EY, EZ};
    do {
        for (int s = 0; s < 8; ++s) {
            int s0 = (s & 1) ? -1 : 1;
            int s1 = (s & 2) ? -1 : 1;
            int s2 = (s & 4) ? -1 : 1;
            Rot R;
            R.m[0] = s0 * e[p[0]];
            R.m[1] = s1 * e[p[1]];
            R.m[2] = s2 * e[p[2]];
            if (s0 * s1 * s2 * parity3(p) == 1) rots.push_back(R);
        }
    } while (next_permutation(p, p + 3));

    for (int r = 0; r < 24; ++r) {
        for (int i = 0; i < 8; ++i) {
            V3 v = applyR(rots[r], V3{PX[i], PY[i], PZ[i]});
            int j = -1;
            for (int t = 0; t < 8; ++t)
                if (PX[t] == v.x && PY[t] == v.y && PZ[t] == v.z) { j = t; break; }
            pmap[r][i] = j;
        }
        for (int id = 0; id < 6; ++id) {
            V3 v = applyR(rots[r], NV[id]);
            int id2 = -1;
            for (int t = 0; t < 6; ++t)
                if (NV[t] == v) { id2 = t; break; }
            nmap[r][id] = id2;
        }
        for (int i = 0; i < 8; ++i) {
            int j = pmap[r][i];
            for (int k = 0; k < 3; ++k) {
                int nid = nmap[r][pn[i][k]];
                int k2 = -1;
                for (int t = 0; t < 3; ++t)
                    if (pn[j][t] == nid) { k2 = t; break; }
                smap[r][i][k] = k2;
            }
        }
    }

    // 六个面顺时针 90°
    base[0].m[0] = EZ; base[0].m[1] = EY; base[0].m[2] = -EX;             // U
    base[1].m[0] = EX; base[1].m[1] = -EZ; base[1].m[2] = EY;             // R
    base[2].m[0] = -EY; base[2].m[1] = EX; base[2].m[2] = EZ;             // F
    base[3].m[0] = -EZ; base[3].m[1] = EY; base[3].m[2] = EX;             // D
    base[4].m[0] = EX; base[4].m[1] = EZ; base[4].m[2] = -EY;             // L
    base[5].m[0] = EY; base[5].m[1] = -EX; base[5].m[2] = EZ;             // B

    for (int f = 0; f < 6; ++f) {
        int mask = 0;
        for (int i = 0; i < 8; ++i) {
            int c = faceAxis[f] == 0 ? PX[i] : (faceAxis[f] == 1 ? PY[i] : PZ[i]);
            if (c == faceSign[f]) mask |= 1 << i;
        }
        layerMask[f] = mask;
    }
}

// ---------------- 几何牌面模拟（用于校验 / 暴力） ----------------
// st[i][k] = 位置 i 的第 k 个槽位上的贴纸“原始法向”（即颜色）
static void applyGeom(int rid, int mask, int st[8][3]) {
    int tmp[8][3];
    memcpy(tmp, st, sizeof(int) * 24);
    for (int i = 0; i < 8; ++i) {
        if (!(mask >> i & 1)) continue;
        int j = pmap[rid][i];
        for (int k = 0; k < 3; ++k) tmp[j][smap[rid][i][k]] = st[i][k];
    }
    memcpy(st, tmp, sizeof(int) * 24);
}

static void toCPCO(const int st[8][3], int cp[8], int co[8]) {
    for (int i = 0; i < 8; ++i) {
        int c = -1;
        for (int c0 = 0; c0 < 8 && c < 0; ++c0) {
            bool ok = true;
            for (int k = 0; k < 3; ++k) {
                bool found = false;
                for (int t = 0; t < 3; ++t) if (pn[c0][t] == st[i][k]) found = true;
                if (!found) { ok = false; break; }
            }
            if (ok) c = c0;
        }
        cp[i] = c;
        int r = -1;
        for (int r0 = 0; r0 < 3 && r < 0; ++r0) {
            bool ok = true;
            for (int k = 0; k < 3; ++k) if (st[i][k] != pn[c][(k + r0) % 3]) ok = false;
            if (ok) r = r0;
        }
        co[i] = r;
    }
}

static void toGeom(const int cp[8], const int co[8], int st[8][3]) {
    for (int i = 0; i < 8; ++i)
        for (int k = 0; k < 3; ++k)
            st[i][k] = pn[cp[i]][(k + co[i]) % 3];
}

// ---------------- 18 个转动表 ----------------
struct Move { int perm[8]; int delta[8]; };
static Move mv[18];
static int mlayer[18][4];              // 每个转动真正移动的 4 个角块位置

static void buildMoves() {
    for (int f = 0; f < 6; ++f) {
        Rot cur; cur.m[0] = EX; cur.m[1] = EY; cur.m[2] = EZ;
        for (int q = 1; q <= 3; ++q) {
            cur = comp(base[f], cur);                 // base^q
            int rid = rotIndex(cur);
            int st[8][3];
            for (int i = 0; i < 8; ++i) for (int k = 0; k < 3; ++k) st[i][k] = pn[i][k];
            applyGeom(rid, layerMask[f], st);
            int cp1[8], co1[8];
            toCPCO(st, cp1, co1);
            int idx = f * 3 + (q - 1);
            int lc = 0;
            for (int p = 0; p < 8; ++p) {
                if (!(layerMask[f] >> p & 1)) {   // 非本层角块不动
                    mv[idx].perm[p] = p;
                    mv[idx].delta[p] = 0;
                    continue;
                }
                int p2 = pmap[rid][p];
                mv[idx].perm[p] = p2;
                mv[idx].delta[p2] = co1[p2];
                mlayer[idx][lc++] = p;
            }
        }
    }
}

static inline void applyMove(const int cp[8], const int co[8], int m, int cp2[8], int co2[8]) {
    memcpy(cp2, cp, sizeof(int) * 8);
    memcpy(co2, co, sizeof(int) * 8);
    for (int t = 0; t < 4; ++t) {
        int p = mlayer[m][t];
        int p2 = mv[m].perm[p];
        cp2[p2] = cp[p];
        co2[p2] = (co[p] + mv[m].delta[p2]) % 3;
    }
}

// ---------------- 整体旋转 / 规范形 ----------------
static int rp[24][8], ro[24][8];
static int rotFor[8][3];

static void buildRotTables() {
    for (int r = 0; r < 24; ++r) {
        int st[8][3];
        for (int i = 0; i < 8; ++i) for (int k = 0; k < 3; ++k) st[i][k] = pn[i][k];
        applyGeom(r, 0xFF, st);
        int cp1[8], co1[8];
        toCPCO(st, cp1, co1);
        for (int i = 0; i < 8; ++i) {
            rp[r][i] = pmap[r][i];
            ro[r][i] = co1[pmap[r][i]];
        }
    }
    for (int pos = 0; pos < 8; ++pos)
        for (int ori = 0; ori < 3; ++ori) {
            rotFor[pos][ori] = -1;
            for (int r = 0; r < 24; ++r) {
                if (rp[r][pos] == 0 && (ori + ro[r][pos]) % 3 == 0) { rotFor[pos][ori] = r; break; }
            }
        }
}

static int fact[7] = {1, 1, 2, 6, 24, 120, 720};
static int pow3[7] = {1, 3, 9, 27, 81, 243, 729};

// 7! 个排列的完美哈希：key = 7 个 3bit 值
static short permRankTab[1 << 21];
static unsigned char unrankTab[5040][8];

static void buildPermTables() {
    memset(permRankTab, 0xFF, sizeof(permRankTab));
    for (int idx = 0; idx < 5040; ++idx) {
        int permIdx = idx;
        bool used[8] = {false};
        int cp[8];
        cp[0] = 0;
        for (int i = 0; i < 7; ++i) {
            int f = fact[6 - i];
            int d = permIdx / f;
            permIdx %= f;
            int val = 0;
            for (int v = 1; v <= 7; ++v) {
                if (used[v]) continue;
                if (d == 0) { val = v; break; }
                --d;
            }
            used[val] = true;
            cp[i + 1] = val;
        }
        int key = 0;
        for (int k = 1; k <= 7; ++k) { key = (key << 3) | cp[k]; unrankTab[idx][k - 1] = (unsigned char)cp[k]; }
        permRankTab[key] = (short)idx;
    }
}

static inline int canonEncode(const int cp[8], const int co[8]) {
    int i0 = 0;
    while (cp[i0] != 0) ++i0;
    int rid = rotFor[i0][co[i0]];
    int cp2[8], co2[8];
    cp2[0] = 0; co2[0] = 0;
    for (int i = 0; i < 8; ++i) {
        if (i == i0) continue;
        int j = rp[rid][i];
        cp2[j] = cp[i];
        co2[j] = (co[i] + ro[rid][i]) % 3;
    }
    int key = 0;
    for (int k = 1; k <= 7; ++k) key = (key << 3) | cp2[k];
    int permIdx = permRankTab[key];
    int oriIdx = 0;
    for (int k = 1; k <= 6; ++k) oriIdx += co2[k] * pow3[k - 1];
    return permIdx * 729 + oriIdx;
}

static void decode(int idx, int cp[8], int co[8]) {
    int permIdx = idx / 729, oriIdx = idx % 729;
    cp[0] = 0; co[0] = 0;
    for (int k = 1; k <= 7; ++k) cp[k] = unrankTab[permIdx][k - 1];
    int sum = 0;
    for (int k = 1; k <= 6; ++k) { co[k] = oriIdx % 3; oriIdx /= 3; sum += co[k]; }
    co[7] = (3 - sum % 3) % 3;
}

// ---------------- BFS ----------------
static unsigned char  dist_[NS];
static unsigned int   cnt_[NS];
static vector<unsigned int> que;

static void precompute() {
    memset(dist_, 255, sizeof(dist_));
    dist_[0] = 0;
    cnt_[0] = 1;
    que.reserve(NS);
    que.push_back(0);
    for (size_t head = 0; head < que.size(); ++head) {
        unsigned int u = que[head];
        int d = dist_[u];
        int cp[8], co[8];
        decode((int)u, cp, co);
        for (int m = 0; m < 18; ++m) {
            int cp2[8], co2[8];
            applyMove(cp, co, m, cp2, co2);
            int v = canonEncode(cp2, co2);
            int dv = dist_[v];
            if (dv == 255) { dist_[v] = d + 1; que.push_back((unsigned)v); dv = d + 1; }
            if (dv == d + 1) {
                unsigned int nv = cnt_[v] + cnt_[u];
                if (nv >= (unsigned)MOD) nv -= MOD;
                cnt_[v] = nv;
            }
        }
    }
}

// ---------------- 工具 ----------------
static const char *TOK[18] = {
    "U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'"
};

static int parseToken(const string &s) {
    static const string faces = "URFDLB";
    size_t pos = faces.find(s[0]);
    if (pos == string::npos) return -1;
    int q = 1;
    if (s.size() > 1) q = (s[1] == '2') ? 2 : 3;
    return (int)pos * 3 + (q - 1);
}

static void runSelfTest() {
    // 1) 几何与表驱动转动一致性
    mt19937 rng(12345);
    int cp[8], co[8];
    for (int t = 0; t < 2000; ++t) {
        for (int i = 0; i < 8; ++i) cp[i] = i;
        for (int i = 0; i < 8; ++i) co[i] = 0;
        int st[8][3];
        // 随机打乱
        for (int s = 0; s < 40; ++s) {
            int m = rng() % 18;
            int nc[8], no[8];
            applyMove(cp, co, m, nc, no);
            memcpy(cp, nc, sizeof(cp));
            memcpy(co, no, sizeof(co));
        }
        toGeom(cp, co, st);
        {
            int cpR[8], coR[8];
            toCPCO(st, cpR, coR);
            for (int i = 0; i < 8; ++i)
                if (cpR[i] != cp[i] || coR[i] != co[i]) {
                    fprintf(stderr, "ROUNDTRIP MISMATCH pos=%d cp %d/%d co %d/%d\n",
                            i, cpR[i], cp[i], coR[i], co[i]);
                    exit(1);
                }
        }
        for (int m = 0; m < 18; ++m) {
            int st2[8][3];
            memcpy(st2, st, sizeof(st2));
            // 生成几何转动
            int f = m / 3, q = m % 3 + 1;
            Rot cur; cur.m[0] = EX; cur.m[1] = EY; cur.m[2] = EZ;
            for (int k = 0; k < q; ++k) cur = comp(base[f], cur);
            int r = rotIndex(cur);
            applyGeom(r, layerMask[f], st2);
            int cpG[8], coG[8];
            toCPCO(st2, cpG, coG);
            int cpT[8], coT[8];
            applyMove(cp, co, m, cpT, coT);
            for (int i = 0; i < 8; ++i)
                if (cpG[i] != cpT[i] || coG[i] != coT[i]) {
                    fprintf(stderr, "MOVE MISMATCH m=%d pos=%d\n", m, i);
                    exit(1);
                }
        }
    }
    fprintf(stderr, "geometry/move-table check: OK\n");

    // 2) 整体旋转表一致性
    for (int t = 0; t < 500; ++t) {
        for (int i = 0; i < 8; ++i) cp[i] = i;
        for (int i = 0; i < 8; ++i) co[i] = 0;
        for (int s = 0; s < 50; ++s) {
            int m = rng() % 18;
            int nc[8], no[8];
            applyMove(cp, co, m, nc, no);
            memcpy(cp, nc, sizeof(cp));
            memcpy(co, no, sizeof(co));
        }
        int st[8][3];
        toGeom(cp, co, st);
        for (int r = 0; r < 24; ++r) {
            int st2[8][3];
            memcpy(st2, st, sizeof(st2));
            applyGeom(r, 0xFF, st2);
            int cpG[8], coG[8];
            toCPCO(st2, cpG, coG);
            int cpT[8], coT[8];
            for (int i = 0; i < 8; ++i) {
                int j = rp[r][i];
                cpT[j] = cp[i];
                coT[j] = (co[i] + ro[r][i]) % 3;
            }
            for (int i = 0; i < 8; ++i)
                if (cpG[i] != cpT[i] || coG[i] != coT[i]) {
                    fprintf(stderr, "ROT MISMATCH r=%d pos=%d\n", r, i);
                    exit(1);
                }
        }
    }
    fprintf(stderr, "global-rotation table check: OK\n");
}

int main(int argc, char **argv) {
    buildGeometry();
    buildMoves();
    buildRotTables();
    buildPermTables();

    bool selfTest = (argc > 1 && string(argv[1]) == "--selftest");
    if (selfTest) runSelfTest();

    auto t0 = chrono::steady_clock::now();
    precompute();
    auto t1 = chrono::steady_clock::now();
    double bt = chrono::duration<double>(t1 - t0).count();

    if (selfTest) {
        long long hist[32] = {0};
        for (int i = 0; i < NS; ++i) if (dist_[i] != 255) hist[dist_[i]]++;
        fprintf(stderr, "BFS time: %.3fs, reached=%zu\n", bt, que.size());
        fprintf(stderr, "distance histogram:\n");
        for (int d = 0; d < 32; ++d) if (hist[d]) fprintf(stderr, "  d=%2d : %lld\n", d, hist[d]);
        static const long long EXPECT[12] = {1, 9, 54, 321, 1847, 9992, 50136, 227536, 870072, 1887748, 623800, 2644};
        bool ok = true;
        for (int d = 0; d < 12; ++d) if (hist[d] != EXPECT[d]) ok = false;
        long long sum = 0;
        for (int d = 0; d < 12; ++d) sum += EXPECT[d];
        fprintf(stderr, "expected sum=%lld actual reached=%zu\n", sum, que.size());
        unsigned int maxc = 0; int argc_ = 0, argd = 0;
        for (int i = 0; i < NS; ++i) if (cnt_[i] > maxc) { maxc = cnt_[i]; argc_ = i; argd = dist_[i]; }
        fprintf(stderr, "max #optimal-solutions = %u (state %d, dist %d)\n", maxc, argc_, argd);
        fprintf(stderr, "distribution check: %s\n", ok && que.size() == (size_t)NS ? "PASS" : "FAIL");
        if (!ok) return 1;
        return 0;
    }

    int T;
    if (!(cin >> T)) return 0;
    string out;
    out.reserve(1 << 20);
    for (int tc = 0; tc < T; ++tc) {
        int k;
        cin >> k;
        int cp[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        int co[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < k; ++i) {
            string s;
            cin >> s;
            int m = parseToken(s);
            int cp2[8], co2[8];
            applyMove(cp, co, m, cp2, co2);
            memcpy(cp, cp2, sizeof(cp));
            memcpy(co, co2, sizeof(co));
        }
        int u = canonEncode(cp, co);
        int d = dist_[u];
        out += to_string(d);
        out += ' ';
        out += to_string(cnt_[u]);
        // 在“真实状态”上按字典序贪心：每步取最小的、能把距离降 1 的转动
        int step = 0;
        while (step < d) {
            for (int m = 0; m < 18; ++m) {
                int a[8], b[8];
                applyMove(cp, co, m, a, b);
                int v = canonEncode(a, b);
                if (dist_[v] == d - step - 1) {
                    out += ' ';
                    out += TOK[m];
                    memcpy(cp, a, sizeof(cp));
                    memcpy(co, b, sizeof(co));
                    break;
                }
            }
            ++step;
        }
        out += '\n';
    }
    fputs(out.c_str(), stdout);
    return 0;
}
