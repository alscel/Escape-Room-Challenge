#include <bits/stdc++.h>
#include <chrono>
#include <thread>

using namespace std;

static inline void clearScreen() {
    // ANSI: clear screen and move cursor to home
    cout << "\x1b[2J\x1b[H";
}

struct Canvas {
    int w, h;
    vector<string> buf;
    Canvas(int W, int H) : w(W), h(H), buf(H, string(W, ' ')) {}
    void put(int x, int y, char c) { if (x >= 0 && x < w && y >= 0 && y < h) buf[y][x] = c; }
    void hline(int x1, int x2, int y, char c) { if (y < 0 || y >= h) return; if (x1 > x2) swap(x1, x2); x1 = max(0, x1); x2 = min(w - 1, x2); for (int x = x1; x <= x2; ++x) buf[y][x] = c; }
    void vline(int x, int y1, int y2, char c) { if (x < 0 || x >= w) return; if (y1 > y2) swap(y1, y2); y1 = max(0, y1); y2 = min(h - 1, y2); for (int y = y1; y <= y2; ++y) buf[y][x] = c; }
    void rect(int x1, int y1, int x2, int y2, char c) { hline(x1, x2, y1, c); hline(x1, x2, y2, c); vline(x1, y1, y2, c); vline(x2, y1, y2, c); }
    void text(int x, int y, const string& s) { for (size_t i = 0; i < s.size(); ++i) put(x + (int)i, y, s[i]); }
    void draw() { for (auto& row : buf) cout << row << "\n"; }
};

void renderDoorFrame(Canvas& cv, int marginLeft, int marginTop, int marginRight, int marginBottom) {
    int x1 = marginLeft;
    int y1 = marginTop;
    int x2 = cv.w - 1 - marginRight;
    int y2 = cv.h - 1 - marginBottom;
    cv.rect(x1, y1, x2, y2, '#');               // outer frame
    cv.rect(x1 + 2, y1 + 2, x2 - 2, y2 - 2, '#');       // inner frame
}

// progress: 0.0 (closed) -> 1.0 (fully open)
void renderSlidingDoubleDoor(Canvas& cv, int left, int top, int right, int bottom, double progress) {
    int innerL = left + 3;
    int innerR = right - 3;
    int innerT = top + 3;
    int innerB = bottom - 3;
    int innerW = max(0, innerR - innerL + 1);
    int innerH = max(0, innerB - innerT + 1);
    if (innerW <= 0 || innerH <= 0) return;

    // Floor base
    cv.hline(innerL - 1, innerR + 1, innerB + 1, '=');

    // Two door panels of fixed width that slide to the sides
    int panelW = max(6, innerW / 6); // visual thickness of each panel
    int maxGap = innerW - 2 * panelW; // center gap when fully open
    if (maxGap < 0) { panelW = max(2, innerW / 4); maxGap = max(0, innerW - 2 * panelW); }

    int gap = (int)round(progress * maxGap);
    int center = (innerL + innerR) / 2;

    // Left door panel [lx1, lx2]
    int lx2 = center - (gap / 2) - 1;      // right edge of left panel
    int lx1 = lx2 - panelW + 1;          // left edge of left panel

    // Right door panel [rx1, rx2]
    int rx1 = center + (gap - gap / 2);    // left edge of right panel
    int rx2 = rx1 + panelW - 1;          // right edge of right panel

    // Clamp inside the inner cavity
    lx1 = max(lx1, innerL); lx2 = min(lx2, innerR);
    rx1 = max(rx1, innerL); rx2 = min(rx2, innerR);

    auto fillPanel = [&](int x1, int x2) {
        for (int y = innerT; y <= innerB; ++y) {
            for (int x = x1; x <= x2; ++x) {
                if (x == x1 || x == x2) cv.put(x, y, '|');
                else cv.put(x, y, '=');
            }
        }
        };

    // Fill door panels
    if (lx1 <= lx2) fillPanel(lx1, lx2);
    if (rx1 <= rx2) fillPanel(rx1, rx2);

    // Door handles (appear near meeting edges when almost closed)
    if (gap < max(6, panelW / 2)) {
        int hy = innerT + innerH / 2;
        if (lx2 >= innerL && lx2 <= innerR) cv.put(lx2 - 1, hy, 'o');
        if (rx1 >= innerL && rx1 <= innerR) cv.put(rx1 + 1, hy, 'o');
    }
}

void renderTitle(Canvas& cv, const string& title) {
    int y = 1;
    int x = max(0, (cv.w - (int)title.size()) / 2);
    for (size_t i = 0; i < title.size(); ++i) { cv.put(x + (int)i, y, title[i]); }
}

void renderSubtitle(Canvas& cv, const string& subtitle) {
    int y = 3;
    int x = max(0, (cv.w - (int)subtitle.size()) / 2);
    for (size_t i = 0; i < subtitle.size(); ++i) { cv.put(x + (int)i, y, subtitle[i]); }
}

void drawFrame(double progress, bool opening, int W = 80, int H = 28) {
    Canvas cv(W, H);

    // Title from user requirement (exact text)
    renderTitle(cv, "Capstone project Escape room");
    renderSubtitle(cv, opening ? "Opening..." : "Closing...");

    // Door frame and panels
    renderDoorFrame(cv, 6, 5, 6, 4);
    renderSlidingDoubleDoor(cv, 6, 5, W - 7, H - 5, progress);

    // Footer hint
    string hint = "Press Ctrl+C to exit";
    int hx = max(0, (W - (int)hint.size()) / 2);
    cv.text(hx, H - 1, hint);

    clearScreen();
    cv.draw();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int W = 90;   // overall canvas width
    const int H = 30;   // overall canvas height
    const int STEPS = 50; // frames per direction
    const int CYCLES = 2; // how many open-close cycles

    for (int cycle = 0; cycle < CYCLES; ++cycle) {
        // Opening
        for (int s = 0; s <= STEPS; ++s) {
            double p = (double)s / STEPS;
            drawFrame(p, true, W, H);
            this_thread::sleep_for(chrono::milliseconds(35));
        }
        this_thread::sleep_for(chrono::milliseconds(400));
        // Closing
        for (int s = STEPS; s >= 0; --s) {
            double p = (double)s / STEPS;
            drawFrame(p, false, W, H);
            this_thread::sleep_for(chrono::milliseconds(35));
        }
        this_thread::sleep_for(chrono::milliseconds(250));
    }

    // Final resting state: closed
    drawFrame(0.0, false, W, H);
    return 0;
}
