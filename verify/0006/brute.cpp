// 0006 独立暴力验证器：O(N^3) 直接检查每段是否可行
#include <cstdio>
#include <vector>
using namespace std;
typedef long long ll;
typedef __int128 i128;

struct Frac { ll num, den; };
static bool fracLess(const Frac &a, const Frac &b) {
    return (i128)a.num * b.den < (i128)b.num * a.den;
}
static bool segOK(const vector<ll> &x, const vector<ll> &z, int s, int e, ll E) {
    bool hxL=false,hxH=false, hzL=false,hzH=false;
    Frac lx{},hx{},lz{},hz{};
    for (int t=s+1;t<=e;++t) {
        ll dt=t-s;
        Frac ax{x[t]-x[s]-E,dt}, bx{x[t]-x[s]+E,dt};
        Frac az{z[t]-z[s]-E,dt}, bz{z[t]-z[s]+E,dt};
        if (!hxL || fracLess(lx,ax)) { lx=ax; hxL=true; }
        if (!hxH || fracLess(bx,hx)) { hx=bx; hxH=true; }
        if (fracLess(hx,lx)) return false;
        if (!hzL || fracLess(lz,az)) { lz=az; hzL=true; }
        if (!hzH || fracLess(bz,hz)) { hz=bz; hzH=true; }
        if (fracLess(hz,lz)) return false;
    }
    return true;
}
int main() {
    int N; ll E;
    if (scanf("%d %lld",&N,&E)!=2) return 0;
    vector<ll> x(N+1), z(N+1);
    for (int i=0;i<=N;++i) scanf("%lld %lld",&x[i],&z[i]);
    const int INF=1e9;
    vector<int> dp(N+1,INF); dp[0]=0;
    for (int e=1;e<=N;++e)
        for (int s=0;s<e;++s)
            if (dp[s]+1<dp[e] && segOK(x,z,s,e,E)) dp[e]=dp[s]+1;
    printf("%d\n", dp[N]);
    return 0;
}
