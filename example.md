# 740. 容易题

> 
> 内存限制：128MB｜时间限制：1000s｜8星
> 解决次数：17｜提交次数：30

## 题目描述

为了使得大家高兴，小Q特意出个自认为的简单题来满足大家，这道简单题是描述如下：

有一个数列 $A$，已知对于所有的 $(A[i])$ 都是 $1 \sim n$ 的自然数，并且知道对于一些 $(A[i])$ 不能取哪些值，我们定义一个数列的积为该数列所有元素的乘积，要求你求出所有可能的数列的积的和 $\bmod 1000000007$ 的值，是不是很简单呢？

## 输入格式

第一行三个整数 $(n,m,k)$ 分别表示数列元素的取值范围，数列元素个数，以及已知的限制条数。
接下来 $k$ 行，每行两个正整数 $(x,y)$ 表示 $(A[x])$ 的值不能是 $y$。

## 输出格式

一行一个整数表示所有可能的数列的积的和对 $1000000007$ 取模后的结果。如果一个合法的数列都没有，答案输出 $0$。

## 样例输入输出

**样例输入**

```
3 4 5
1 1
1 1
2 2
2 3
4 3
```

**样例输出**

```
90
```

## 数据范围

对于 $100%$ 的数据，保证 $n \le 10^9,m \le 10^9,k \le 10^5,1 \le y \le n,1 \le x \le m$。

## 解题思路

设 $S = 1+2+\dots+n = \dfrac{n(n+1)}{2} \pmod{MOD}$。

- 如果某个位置 $x$ 没有任何限制：该位置可选值贡献和为 $S$；
- 如果某个位置 $x$ 有禁止集合 $ban$：该位置贡献和为 $(S - \sum ban_y) \pmod{MOD}$；> 
> 注意：同一个 $((x,y))$ 限制会重复输入，需要去重。

设：

- $cnt$：有**限制**的不同位置数量；
- $prod$：所有有限制位置的贡献乘积；
剩下 $m-cnt$ 个无限制位置，每个贡献 $S$，等价于快速幂 $S^{m-cnt}$。

总答案：
$$
ans = prod \times pow(S,\ m-cnt,\ MOD) \pmod{MOD}
$$
如果某个有限制位置贡献变成 $0$，整体答案就是 $0$。

> 
> MOD = 1e9+7 是质数。

## C++ 代码

```
#include <iostream>
#include <map>
#include <unordered_set>
using namespace std;
typedef long long ll;
const int MOD = 1000000007;

ll qpow(ll a, ll b) {
    ll res = 1;
    while(b) {
        if(b & 1) res = res * a % MOD;
        a = a * a % MOD;
        b >>= 1;
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    ll n, m;
    int k;
    cin >> n >> m >> k;
    map<ll, unordered_set<ll>> ban;
    for(int i = 0; i < k; i++) {
        ll x, y;
        cin >> x >> y;
        ban[x].insert(y);
    }
    // S = n*(n+1)/2 mod MOD
    ll S = n % MOD;
    S = S * ((n + 1) % MOD) % MOD;
    ll inv2 = qpow(2, MOD - 2);
    S = S * inv2 % MOD;

    ll prod = 1;
    for(auto &p : ban) {
        ll sum_del = 0;
        for(ll v : p.second) {
            sum_del = (sum_del + v % MOD) % MOD;
        }
        ll now = (S - sum_del + MOD) % MOD;
        prod = prod * now % MOD;
    }
    ll free = m - ban.size();
    ll ans = prod * qpow(S, free) % MOD;
    cout << ans << endl;
    return 0;
}
```

## Python 代码

```
MOD = 1000000007

def pow_mod(a, b):
    return pow(a, b, MOD)

def main():
    import sys
    input = sys.stdin.read().split()
    ptr = 0
    n = int(input[ptr]); ptr +=1
    m = int(input[ptr]); ptr +=1
    k = int(input[ptr]); ptr +=1

    from collections import defaultdict
    ban = defaultdict(set)
    for _ in range(k):
        x = int(input[ptr]); ptr +=1
        y = int(input[ptr]); ptr +=1
        ban[x].add(y)
    
    # S = n(n+1)/2 mod MOD
    S = (n % MOD) * ((n + 1) % MOD) % MOD
    inv2 = pow_mod(2, MOD - 2)
    S = S * inv2 % MOD

    prod = 1
    for st in ban.values():
        sdel = 0
        for v in st:
            sdel = (sdel + v % MOD) % MOD
        cur = (S - sdel + MOD) % MOD
        prod = prod * cur % MOD
    
    free = m - len(ban)
    ans = prod * pow_mod(S, free) % MOD
    print(ans)

if __name__ == "__main__":
    main()
```

---

需要我把这题也整理成和上一题一样完整的 md 文档吗？