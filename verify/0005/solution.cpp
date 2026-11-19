// 0005 - PCG64 随机流的 O(log K) 跳转（128 位 LCG 仿射幂）
// 参考项目：E:\project\musicclone\moss-tts-cpp（main.cpp 的 next_random_ / NumPy PCG64 兼容实现）
// 编译：g++ -O2 -std=c++17 -o solution solution.cpp
#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;

struct U128 { ull lo, hi; };

// 64x64 -> 128
static inline void mul64(ull a, ull b, ull &lo, ull &hi) {
    ull a0 = a & 0xffffffffULL, a1 = a >> 32;
    ull b0 = b & 0xffffffffULL, b1 = b >> 32;
    ull p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    ull mid = (p00 >> 32) + (p01 & 0xffffffffULL) + (p10 & 0xffffffffULL);
    lo = (mid << 32) | (p00 & 0xffffffffULL);
    hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}

// (a*b) mod 2^128
static inline U128 mul128(U128 a, U128 b) {
    ull lo, hi0, lo2, hi2, lo3, hi3;
    mul64(a.lo, b.lo, lo, hi0);
    mul64(a.hi, b.lo, lo2, hi2);   // 只取低 64 位，高 64 位溢出 128 位
    mul64(a.lo, b.hi, lo3, hi3);
    return {lo, hi0 + lo2 + lo3};
}

static inline U128 add128(U128 a, U128 b) {
    ull lo = a.lo + b.lo;
    ull hi = a.hi + b.hi + (lo < b.lo ? 1 : 0);
    return {lo, hi};
}

// 项目常量（NumPy default_rng(1234) 导出的 PCG64 状态）
static const U128 MULT = {0x4385df649fccf645ULL, 0x2360ed051fc65da4ULL};
static const U128 INC  = {0xd0f51ce6006e4325ULL, 0x50c8fb163c7cea4eULL};
static const U128 S0   = {0xf69b873d9fe45409ULL, 0x160ad84006fe21eaULL};

// 仿射映射 x -> a*x + c
struct Aff { U128 a, c; };

// 先应用 A 再应用 B
static inline Aff compose(const Aff &A, const Aff &B) {
    return {mul128(B.a, A.a), add128(mul128(B.a, A.c), B.c)};
}

// PCG64 XSL RR 64 输出
static inline ull output_of(U128 s) {
    ull x = s.hi ^ s.lo;
    unsigned r = (unsigned)(s.hi >> 58);
    if (r) x = (x >> r) | (x << (64 - r));
    return x;
}

static inline void appendUInt(string &out, ull v) {
    char buf[24];
    int n = 0;
    if (v == 0) buf[n++] = '0';
    while (v) { buf[n++] = char('0' + int(v % 10)); v /= 10; }
    while (n) out.push_back(buf[--n]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // powA[b] = A^(2^b)，A 为一步 LCG：x -> MULT*x + INC
    const int B = 62;
    vector<Aff> powA(B);
    powA[0] = {MULT, INC};
    for (int b = 1; b < B; ++b) powA[b] = compose(powA[b - 1], powA[b - 1]);

    int Q;
    cin >> Q;
    string out;
    out.reserve((size_t)Q * 18);
    for (int q = 0; q < Q; ++q) {
        ull K;
        cin >> K;
        ull steps = K + 1;   // 第 K 个输出是「先步进、后取输出」
        Aff res = {{1, 0}, {0, 0}};
        for (int b = 0; b < B; ++b)
            if ((steps >> b) & 1) res = compose(res, powA[b]);
        U128 state = add128(mul128(res.a, S0), res.c);
        ull val = output_of(state) >> 11;   // 53 位随机数，即项目里 double 的分子
        appendUInt(out, val);
        out.push_back('\n');
    }
    fputs(out.c_str(), stdout);
    return 0;
}
