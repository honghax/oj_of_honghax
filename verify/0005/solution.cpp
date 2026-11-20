// 0005 - NumPy default_rng(seed) 复刻：SeedSequence + PCG64 跳转
// 参考项目：E:\project\musicclone\moss-tts-cpp（main.cpp 的 NumPy PCG64 兼容实现）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
using namespace std;
typedef uint32_t u32;
typedef uint64_t u64;

struct U128 { u64 lo, hi; };

#if defined(__SIZEOF_INT128__)
using u128 = unsigned __int128;
static inline U128 mul128(U128 a, U128 b) {
    u128 x = ((u128)a.hi << 64 | a.lo) * ((u128)b.hi << 64 | b.lo);
    return {(u64)x, (u64)(x >> 64)};
}
#else
static inline void mul64(u64 a, u64 b, u64 &lo, u64 &hi) {
    u64 a0 = a & 0xffffffffULL, a1 = a >> 32;
    u64 b0 = b & 0xffffffffULL, b1 = b >> 32;
    u64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    u64 mid = (p00 >> 32) + (p01 & 0xffffffffULL) + (p10 & 0xffffffffULL);
    lo = (mid << 32) | (p00 & 0xffffffffULL);
    hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}
static inline U128 mul128(U128 a, U128 b) {
    u64 lo, hi0, lo2, hi2, lo3, hi3;
    mul64(a.lo, b.lo, lo, hi0);
    mul64(a.hi, b.lo, lo2, hi2);
    mul64(a.lo, b.hi, lo3, hi3);
    return {lo, hi0 + lo2 + lo3};
}
#endif

static inline U128 add128(U128 a, U128 b) {
    u64 lo = a.lo + b.lo;
    return {lo, a.hi + b.hi + (lo < b.lo)};
}

static const U128 MULT = {0x4385df649fccf645ULL, 0x2360ed051fc65da4ULL};

static inline u32 hashmix(u32 v, u32 &hc) {
    v ^= hc;
    hc *= 0x931e8875u;
    v *= hc;
    v ^= v >> 16;
    return v;
}
static inline u32 mix32(u32 x, u32 y) {
    u32 r = 0xca01f9ddu * x - 0x4973f715u * y;
    r ^= r >> 16;
    return r;
}

// NumPy SeedSequence(seed)，pool_size=4，返回 PCG64 的初始 state 与 inc
static void seedseq(u64 seed, U128 &S0, U128 &INC) {
    vector<u32> ent;
    if (seed == 0) ent.push_back(0);
    else {
        u64 n = seed;
        while (n) { ent.push_back((u32)(n & 0xffffffffULL)); n >>= 32; }
    }

    u32 pool[4];
    u32 hc = 0x43b0d7e5u;                       // INIT_A
    for (int i = 0; i < 4; ++i)
        pool[i] = hashmix(i < (int)ent.size() ? ent[i] : 0u, hc);
    for (int src = 0; src < 4; ++src)
        for (int dst = 0; dst < 4; ++dst)
            if (src != dst) pool[dst] = mix32(pool[dst], hashmix(pool[src], hc));
    for (int src = 4; src < (int)ent.size(); ++src)
        for (int dst = 0; dst < 4; ++dst)
            pool[dst] = mix32(pool[dst], hashmix(ent[src], hc));

    u32 words[8];
    hc = 0x8b51f9ddu;                           // INIT_B
    for (int i = 0; i < 8; ++i) {
        u32 v = pool[i & 3];
        v ^= hc;
        hc *= 0x58f38dedu;                      // MULT_B
        v *= hc;
        v ^= v >> 16;
        words[i] = v;
    }
    u64 val[4];
    for (int j = 0; j < 4; ++j) val[j] = (u64)words[2 * j] | ((u64)words[2 * j + 1] << 32);

    U128 initstate = {val[1], val[0]};
    U128 initseq = {val[3], val[2]};
    INC = {(initseq.lo << 1) | 1u, (initseq.hi << 1) | (initseq.lo >> 63)};

    // pcg_setseq_128_srandom: state=0; step; state+=initstate; step
    U128 s = {0, 0};
    s = add128(mul128(s, MULT), INC);
    s = add128(s, initstate);
    s = add128(mul128(s, MULT), INC);
    S0 = s;
}

static inline u64 output_of(U128 s) {
    u64 x = s.hi ^ s.lo;
    unsigned r = (unsigned)(s.hi >> 58);
    if (r) x = (x >> r) | (x << (64 - r));
    return x;
}

static inline void appendUInt(string &out, u64 v) {
    char buf[24];
    int n = 0;
    if (v == 0) buf[n++] = '0';
    while (v) { buf[n++] = char('0' + int(v % 10)); v /= 10; }
    while (n) out.push_back(buf[--n]);
}

int main() {
    int Q;
    if (scanf("%d", &Q) != 1) return 0;
    string out;
    out.reserve((size_t)Q * 18);
    for (int qi = 0; qi < Q; ++qi) {
        unsigned long long seed, K;
        scanf("%llu %llu", &seed, &K);
        U128 S0, INC;
        seedseq(seed, S0, INC);

        // 该 seed 的仿射映射 A(x)=MULT*x+INC 的 2^b 次幂
        struct Aff { U128 a, c; };
        auto comp = [](const Aff &A, const Aff &B) {
            return Aff{mul128(B.a, A.a), add128(mul128(B.a, A.c), B.c)};
        };
        Aff pw[64];
        pw[0] = {MULT, INC};
        for (int b = 1; b < 64; ++b) pw[b] = comp(pw[b - 1], pw[b - 1]);

        unsigned long long steps = K + 1;   // 第 K 个输出 = 步进 K+1 次后取值
        Aff res = {{1, 0}, {0, 0}};
        for (int b = 0; b < 64; ++b)
            if ((steps >> b) & 1) res = comp(res, pw[b]);
        U128 st = add128(mul128(res.a, S0), res.c);
        appendUInt(out, output_of(st) >> 11);
        out.push_back('\n');
    }
    fputs(out.c_str(), stdout);
    return 0;
}
