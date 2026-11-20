// 0005 独立暴力验证器：SeedSequence + 顺序步进（只适合小 K）
#include <cstdio>
#include <cstdint>
#include <vector>
using namespace std;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned __int128 u128;

struct U128 { u64 lo, hi; };
static inline U128 mul128(U128 a, U128 b) {
    u128 x = ((u128)a.hi << 64 | a.lo) * ((u128)b.hi << 64 | b.lo);
    return {(u64)x, (u64)(x >> 64)};
}
static inline U128 add128(U128 a, U128 b) {
    u64 lo = a.lo + b.lo;
    return {lo, a.hi + b.hi + (lo < b.lo)};
}
static const U128 MULT = {0x4385df649fccf645ULL, 0x2360ed051fc65da4ULL};
static inline u32 hashmix(u32 v, u32 &hc) {
    v ^= hc; hc *= 0x931e8875u; v *= hc; v ^= v >> 16; return v;
}
static inline u32 mix32(u32 x, u32 y) {
    u32 r = 0xca01f9ddu * x - 0x4973f715u * y; r ^= r >> 16; return r;
}
static void seedseq(u64 seed, U128 &S0, U128 &INC) {
    vector<u32> ent;
    if (seed == 0) ent.push_back(0);
    else { u64 n = seed; while (n) { ent.push_back((u32)(n & 0xffffffffULL)); n >>= 32; } }
    u32 pool[4], hc = 0x43b0d7e5u;
    for (int i = 0; i < 4; ++i) pool[i] = hashmix(i < (int)ent.size() ? ent[i] : 0u, hc);
    for (int s = 0; s < 4; ++s) for (int d = 0; d < 4; ++d)
        if (s != d) pool[d] = mix32(pool[d], hashmix(pool[s], hc));
    for (int s = 4; s < (int)ent.size(); ++s) for (int d = 0; d < 4; ++d)
        pool[d] = mix32(pool[d], hashmix(ent[s], hc));
    u32 w[8]; hc = 0x8b51f9ddu;
    for (int i = 0; i < 8; ++i) {
        u32 v = pool[i & 3]; v ^= hc; hc *= 0x58f38dedu; v *= hc; v ^= v >> 16; w[i] = v;
    }
    u64 val[4];
    for (int j = 0; j < 4; ++j) val[j] = (u64)w[2*j] | ((u64)w[2*j+1] << 32);
    U128 initstate = {val[1], val[0]}, initseq = {val[3], val[2]};
    INC = {(initseq.lo << 1) | 1u, (initseq.hi << 1) | (initseq.lo >> 63)};
    U128 s = {0, 0};
    s = add128(mul128(s, MULT), INC);
    s = add128(s, initstate);
    s = add128(mul128(s, MULT), INC);
    S0 = s;
}
static inline u64 output_of(U128 s) {
    u64 x = s.hi ^ s.lo; unsigned r = (unsigned)(s.hi >> 58);
    if (r) x = (x >> r) | (x << (64 - r));
    return x;
}
int main() {
    int Q; scanf("%d", &Q);
    for (int i = 0; i < Q; ++i) {
        u64 seed, K; scanf("%llu %llu", &seed, &K);
        U128 s, inc; seedseq(seed, s, inc);
        for (u64 k = 0; k <= K; ++k) s = add128(mul128(s, MULT), inc);
        printf("%llu\n", output_of(s) >> 11);
    }
    return 0;
}
