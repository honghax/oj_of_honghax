// 0001 独立暴力验证器：几何牌面 + 字典序 DFS 枚举
// 仅用于小深度对拍，输出格式与 solution 相同
#include <bits/stdc++.h>
using namespace std;

struct V3 { int x, y, z; };
static inline V3 operator+(V3 a, V3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline V3 operator-(V3 a) { return {-a.x, -a.y, -a.z}; }
static inline V3 operator*(int k, V3 a) { return {k * a.x, k * a.y, k * a.z}; }
static inline bool operator==(V3 a, V3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
static const V3 EX{1,0,0}, EY{0,1,0}, EZ{0,0,1};
struct Rot { V3 m[3]; };
static inline V3 applyR(const Rot &r, V3 v) { return (v.x * r.m[0]) + (v.y * r.m[1]) + (v.z * r.m[2]); }
static inline Rot comp(const Rot &A, const Rot &B) { Rot C; for (int i = 0; i < 3; ++i) C.m[i] = applyR(A, B.m[i]); return C; }

static const int PX[8] = { 1,-1,-1, 1, 1,-1,-1, 1};
static const int PY[8] = { 1, 1, 1, 1,-1,-1,-1,-1};
static const int PZ[8] = { 1, 1,-1,-1, 1, 1,-1,-1};
static const V3 NV[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};

static int pn[8][3];
static Rot base[6];
static int layerMask[6];
static int pmap[24][8], smap[24][8][3];
static vector<Rot> rots;

static int parity3(const int p[3]) {
    int inv = 0;
    for (int i = 0; i < 3; ++i) for (int j = i + 1; j < 3; ++j) if (p[i] > p[j]) ++inv;
    return (inv & 1) ? -1 : 1;
}
static int rotIndex(const Rot &r) {
    for (int i = 0; i < 24; ++i) {
        bool ok = true;
        for (int k = 0; k < 3 && ok; ++k) if (!(rots[i].m[k] == r.m[k])) ok = false;
        if (ok) return i;
    }
    return -1;
}
static void build() {
    for (int i = 0; i < 8; ++i) {
        int xid = PX[i] > 0 ? 0 : 1, yid = PY[i] > 0 ? 2 : 3, zid = PZ[i] > 0 ? 4 : 5;
        if (PX[i] * PY[i] * PZ[i] < 0) swap(yid, zid);
        pn[i][0] = xid; pn[i][1] = yid; pn[i][2] = zid;
    }
    int p[3] = {0,1,2}; V3 e[3] = {EX, EY, EZ};
    do {
        for (int s = 0; s < 8; ++s) {
            int s0 = (s & 1) ? -1 : 1, s1 = (s & 2) ? -1 : 1, s2 = (s & 4) ? -1 : 1;
            Rot R;
            R.m[0] = s0 * e[p[0]]; R.m[1] = s1 * e[p[1]]; R.m[2] = s2 * e[p[2]];
            if (s0 * s1 * s2 * parity3(p) == 1) rots.push_back(R);
        }
    } while (next_permutation(p, p + 3));
    for (int r = 0; r < 24; ++r) {
        for (int i = 0; i < 8; ++i) {
            V3 v = applyR(rots[r], V3{PX[i], PY[i], PZ[i]});
            int j = -1;
            for (int t = 0; t < 8; ++t) if (PX[t] == v.x && PY[t] == v.y && PZ[t] == v.z) { j = t; break; }
            pmap[r][i] = j;
        }
        for (int i = 0; i < 8; ++i) {
            int j = pmap[r][i];
            for (int k = 0; k < 3; ++k) {
                V3 v = applyR(rots[r], NV[pn[i][k]]);
                int nid = -1;
                for (int t = 0; t < 6; ++t) if (NV[t] == v) { nid = t; break; }
                int k2 = -1;
                for (int t = 0; t < 3; ++t) if (pn[j][t] == nid) { k2 = t; break; }
                smap[r][i][k] = k2;
            }
        }
    }
    base[0].m[0] = EZ; base[0].m[1] = EY; base[0].m[2] = -EX;
    base[1].m[0] = EX; base[1].m[1] = -EZ; base[1].m[2] = EY;
    base[2].m[0] = -EY; base[2].m[1] = EX; base[2].m[2] = EZ;
    base[3].m[0] = -EZ; base[3].m[1] = EY; base[3].m[2] = EX;
    base[4].m[0] = EX; base[4].m[1] = EZ; base[4].m[2] = -EY;
    base[5].m[0] = EY; base[5].m[1] = -EX; base[5].m[2] = EZ;
    int faceAxis[6] = {1,0,2,1,0,2}, faceSign[6] = {1,1,1,-1,-1,-1};
    for (int f = 0; f < 6; ++f) {
        int mask = 0;
        for (int i = 0; i < 8; ++i) {
            int c = faceAxis[f] == 0 ? PX[i] : (faceAxis[f] == 1 ? PY[i] : PZ[i]);
            if (c == faceSign[f]) mask |= 1 << i;
        }
        layerMask[f] = mask;
    }
}

static int moveRid[18];
static void buildMoves() {
    for (int f = 0; f < 6; ++f) {
        Rot cur; cur.m[0] = EX; cur.m[1] = EY; cur.m[2] = EZ;
        for (int q = 1; q <= 3; ++q) {
            cur = comp(base[f], cur);
            moveRid[f * 3 + q - 1] = rotIndex(cur);
        }
    }
}

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

static bool uniform(const int st[8][3]) {
    for (int f = 0; f < 6; ++f) {
        int color = -1;
        for (int i = 0; i < 8; ++i)
            for (int k = 0; k < 3; ++k)
                if (pn[i][k] == f) {
                    if (color < 0) color = st[i][k];
                    else if (color != st[i][k]) return false;
                }
    }
    return true;
}

static int parseToken(const string &s) {
    static const string faces = "URFDLB";
    size_t pos = faces.find(s[0]);
    int q = 1;
    if (s.size() > 1) q = (s[1] == '2') ? 2 : 3;
    return (int)pos * 3 + (q - 1);
}

static int limitDepth;
static long long foundCount;
static bool foundFirst;
static int bestSeq[32];
static int curSeq[32];

static void dfs(int st[8][3], int depth, int maxDepth) {
    if (depth == maxDepth) return;
    for (int m = 0; m < 18; ++m) {
        if (depth > 0 && m == (curSeq[depth - 1] / 3) * 3 + (2 - curSeq[depth - 1] % 3)) continue; // 剪掉逆着往回走
        int st2[8][3];
        memcpy(st2, st, sizeof(int) * 24);
        int f = m / 3;
        applyGeom(moveRid[m], layerMask[f], st2);
        curSeq[depth] = m;
        if (depth + 1 == maxDepth) {
            if (uniform(st2)) {
                if (!foundFirst) {
                    foundFirst = true;
                    memcpy(bestSeq, curSeq, sizeof(int) * maxDepth);
                }
                ++foundCount;
            }
        } else {
            dfs(st2, depth + 1, maxDepth);
        }
    }
}

int main(int argc, char **argv) {
    build();
    buildMoves();
    if (argc > 1 && string(argv[1]) == "--check") {
        // 校验：每行 k 个打乱 + L 个解，检查最终是否每面同色
        int T;
        if (!(cin >> T)) return 0;
        while (T--) {
            int k;
            cin >> k;
            int st[8][3];
            for (int i = 0; i < 8; ++i) for (int j = 0; j < 3; ++j) st[i][j] = pn[i][j];
            for (int i = 0; i < k; ++i) { string s; cin >> s; int m = parseToken(s); applyGeom(moveRid[m], layerMask[m / 3], st); }
            int L;
            cin >> L;
            for (int i = 0; i < L; ++i) { string s; cin >> s; int m = parseToken(s); applyGeom(moveRid[m], layerMask[m / 3], st); }
            printf("%s\n", uniform(st) ? "OK" : "FAIL");
        }
        return 0;
    }
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int k;
        cin >> k;
        int st[8][3];
        for (int i = 0; i < 8; ++i) for (int j = 0; j < 3; ++j) st[i][j] = pn[i][j];
        for (int i = 0; i < k; ++i) {
            string s;
            cin >> s;
            int m = parseToken(s);
            applyGeom(moveRid[m], layerMask[m / 3], st);
        }
        int ansL = -1;
        for (int L = 0; L <= 6; ++L) {
            if (L == 0) {
                if (uniform(st)) { ansL = 0; foundCount = 1; foundFirst = true; break; }
                continue;
            }
            foundCount = 0; foundFirst = false;
            dfs(st, 0, L);
            if (foundCount > 0) { ansL = L; break; }
        }
        if (ansL < 0) { printf("DEEP\n"); continue; }
        printf("%d %lld", ansL, foundCount);
        static const char *TOK[18] = {"U","U2","U'","R","R2","R'","F","F2","F'","D","D2","D'","L","L2","L'","B","B2","B'"};
        if (ansL > 0) for (int i = 0; i < ansL; ++i) printf(" %s", TOK[bestSeq[i]]);
        printf("\n");
    }
    return 0;
}
