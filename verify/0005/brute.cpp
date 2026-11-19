// 0005 独立暴力验证器：顺序步进 PCG64（只适合小 K）
#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;

struct U128 { ull lo, hi; };
static const U128 MULT = {0x4385df649fccf645ULL, 0x2360ed051fc65da4ULL};
static const U128 INC  = {0xd0f51ce6006e4325ULL, 0x50c8fb163c7cea4eULL};
static const U128 S0   = {0xf69b873d9fe45409ULL, 0x160ad84006fe21eaULL};

static inline void mul64(ull a, ull b, ull &lo, ull &hi) {
    ull a0 = a & 0xffffffffULL, a1 = a >> 32;
    ull b0 = b & 0xffffffffULL, b1 = b >> 32;
    ull p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    ull mid = (p00 >> 32) + (p01 & 0xffffffffULL) + (p10 & 0xffffffffULL);
    lo = (mid << 32) | (p00 & 0xffffffffULL);
    hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}
static inline U128 mul128(U128 a, U128 b) {
    ull lo, hi0, lo2, hi2, lo3, hi3;
    mul64(a.lo, b.lo, lo, hi0);
    mul64(a.hi, b.lo, lo2, hi2);
    mul64(a.lo, b.hi, lo3, hi3);
    return {lo, hi0 + lo2 + lo3};
}
static inline U128 add128(U128 a, U128 b) {
    ull lo = a.lo + b.lo;
    return {lo, a.hi + b.hi + (lo < b.lo ? 1 : 0)};
}
static inline ull output_of(U128 s) {
    ull x = s.hi ^ s.lo;
    unsigned r = (unsigned)(s.hi >> 58);
    if (r) x = (x >> r) | (x << (64 - r));
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int Q;
    cin >> Q;
    vector<ull> ks(Q);
    ull mx = 0;
    for (int i = 0; i < Q; ++i) { cin >> ks[i]; mx = max(mx, ks[i]); }

    vector<ull> val(mx + 1);
    U128 s = S0;
    for (ull k = 0; k <= mx; ++k) {
        s = add128(mul128(s, MULT), INC);
        val[k] = output_of(s) >> 11;
    }
    for (ull k : ks) cout << val[k] << '\n';
    return 0;
}
