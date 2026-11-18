// 0002 独立暴力验证器：逐询问扫描全部岸格
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, DX, DY;
    if (!(cin >> W >> H >> DX >> DY)) return 0;
    vector<string> mask(H);
    for (int i = 0; i < H; ++i) cin >> mask[i];

    int Q;
    cin >> Q;
    string out;
    while (Q--) {
        int r, c;
        cin >> r >> c;
        ll best = LLONG_MAX;
        int br = -1, bc = -1;
        for (int i = 0; i < H; ++i) {
            for (int j = 0; j < W; ++j) {
                if (mask[i][j] != '0') continue;
                ll dy = (ll)DY * (r - i);
                ll dx = (ll)DX * (c - j);
                ll d2 = dy * dy + dx * dx;
                if (d2 < best || (d2 == best && (i < br || (i == br && j < bc)))) {
                    best = d2;
                    br = i;
                    bc = j;
                }
            }
        }
        if (br < 0) out += "-1 -1 -1\n";
        else {
            out += to_string(br);
            out += ' ';
            out += to_string(bc);
            out += ' ';
            out += to_string(best);
            out += '\n';
        }
    }
    fputs(out.c_str(), stdout);
    return 0;
}
