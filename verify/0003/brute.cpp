// 0003 独立暴力验证器：逐标签与已放置矩形做 O(N^2) 检查
#include <bits/stdc++.h>
using namespace std;

struct Rect { int x, y, w, h; };

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, N;
    if (!(cin >> W >> H >> N)) return 0;

    vector<Rect> placed;
    string out;
    for (int i = 0; i < N; ++i) {
        int x, y, w, h;
        cin >> x >> y >> w >> h;
        bool ok = false;
        int fy = y;
        for (int attempt = 0; attempt < 3 && !ok; ++attempt) {
            const int yy = y - attempt * (h + 5);
            bool hit = false;
            for (const Rect &r : placed) {
                if (x < r.x + r.w && x + w > r.x && yy < r.y + r.h && yy + h > r.y) {
                    hit = true;
                    break;
                }
            }
            if (!hit) {
                placed.push_back({x, yy, w, h});
                fy = yy;
                ok = true;
            }
        }
        if (ok) {
            out += "1 " + to_string(x) + " " + to_string(fy) + "\n";
        } else {
            out += "0\n";
        }
    }
    fputs(out.c_str(), stdout);
    return 0;
}
