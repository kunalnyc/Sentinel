#include "login.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "../security/sha256.h"
#include <stdint.h>

// ── Password config ───────────────────────────────────────────────────
// Default password: "sentinel"
// SHA-256 of "sentinel":
// Change this hash to change the password
static const unsigned char PASSWORD_HASH[32] = {
    0x2b, 0x78, 0x47, 0xb7, 0xb7, 0x05, 0x78, 0x1d,
    0x7c, 0xf2, 0x1a, 0x05, 0xe9, 0xc1, 0xbb, 0x37,
    0xcb, 0xf0, 0x78, 0xae, 0xa1, 0x03, 0xbc, 0x3e,
    0xdc, 0xc6, 0xac, 0xa5, 0x2a, 0xb6, 0x54, 0x53
};
// ── Scaled font renderer ──────────────────────────────────────────────
// CS Theory: Nearest-neighbor scaling — each pixel becomes scale×scale block
// Time complexity: O(chars × 8 × 8 × scale²)
static void draw_char_scaled(char c, int x, int y, uint32_t color, int scale)
{
    extern uint8_t font_8x8_basic[256][8];
    uint8_t *glyph = font_8x8_basic[(unsigned char)c];

    int row, col, sy, sx;
    for(row = 0; row < 8; row++)
    {
        for(col = 0; col < 8; col++)
        {
            if(glyph[row] & (1 << (7 - col)))
            {
                // Draw scale×scale block for each pixel
                for(sy = 0; sy < scale; sy++)
                    for(sx = 0; sx < scale; sx++)
                        draw_pixel(x + col*scale + sx,
                                   y + row*scale + sy,
                                   color);
            }
        }
    }
}

static void draw_string_scaled(int x, int y, const char *str,
                                uint32_t color, int scale)
{
    while(*str)
    {
        draw_char_scaled(*str, x, y, color, scale);
        x += 8 * scale + scale; // char width + spacing
        str++;
    }
}

// Center a string horizontally
static int center_x(const char *str, int scale, int screen_w)
{
    int len = 0;
    while(str[len]) len++;
    int text_w = len * (8 * scale + scale);
    return (screen_w - text_w) / 2;
}

// ── Color helpers ─────────────────────────────────────────────────────
#define COL_BG          0x0D0D0D   // near black
#define COL_WHITE       0xFFFFFF
#define COL_GRAY        0x888888
#define COL_LIGHTGRAY   0xCCCCCC
#define COL_GOLD        0xFFC832
#define COL_GREEN       0x00FF64
#define COL_RED         0xFF4444
#define COL_BLUE        0x4A9EFF
#define COL_DARKGRAY    0x222222
#define COL_PANEL       0x1A1A2E
#define COL_BORDER      0x333355

// ── Draw gradient background ──────────────────────────────────────────
static void draw_gradient_bg(int w, int h)
{
    int y;
    for(y = 0; y < h; y++)
    {
        // Interpolate from dark blue-black at top to pure black at bottom
        uint32_t r = 0x0D;
        uint32_t g = 0x0D + (y * 0x05) / h;
        uint32_t b = 0x1A + (y * 0x10) / h;
        uint32_t col = (r << 16) | (g << 8) | b;
        draw_line(0, y, w, y, col);
    }
}

// ── Draw centered rounded panel ───────────────────────────────────────
static void draw_panel(int cx, int cy, int pw, int ph)
{
    int x = cx - pw/2;
    int y = cy - ph/2;
    // Panel background
    draw_rounded_rect(x, y, pw, ph, 12, COL_PANEL);
    // Panel border
    draw_rounded_rect(x+1, y+1, pw-2, ph-2, 11, COL_BORDER);
    draw_rounded_rect(x+2, y+2, pw-4, ph-4, 10, COL_PANEL);
}

// ── Draw password input box ───────────────────────────────────────────
static void draw_input_box(int cx, int y, int w, int h,
                           const char *mask, int focused, int error)
{
    int x = cx - w/2;
    uint32_t border_col = error   ? COL_RED  :
                          focused ? COL_GOLD  : COL_BORDER;

    // Box background
    draw_rect(x, y, w, h, 0x111122);
    // Border
    draw_rect(x,   y,   w,   1, border_col);
    draw_rect(x,   y+h-1, w, 1, border_col);
    draw_rect(x,   y,   1,   h, border_col);
    draw_rect(x+w-1, y, 1,   h, border_col);

    // Draw dots for each character entered
    if(mask)
    {
        int i = 0;
        int dot_x = x + 16;
        while(mask[i] && i < 32)
        {
            draw_rect(dot_x, y + h/2 - 3, 6, 6, COL_WHITE);
            dot_x += 14;
            i++;
        }
        // Blinking cursor
        draw_rect(dot_x, y + h/2 - 6, 2, 12, COL_GOLD);
    }
}

// ── Draw system time placeholder ──────────────────────────────────────
static void draw_clock(int w)
{
    // Static display — real clock needs RTC driver (Day 16)
    draw_string_scaled(center_x("12:00", 2, w),
                       20, "12:00", COL_WHITE, 2);
}

// ── Draw language bar (like macOS) ────────────────────────────────────
static void draw_language_bar(int w, int h)
{
    // Bottom bar
    draw_rect(0, h-30, w, 30, 0x111111);
    draw_line(0, h-30, w, h-30, COL_BORDER);

    // Language indicators
    const char *langs[] = {"EN", "HI", "JA", "KO", "ZH"};
    int lx = 20;
    int li;
    for(li = 0; li < 5; li++)
    {
        uint32_t col = li == 0 ? COL_GOLD : COL_GRAY;
        draw_string_scaled(lx, h-22, langs[li], col, 1);
        lx += 30;
    }

    // Right side — OS name
    draw_string_scaled(w - 120, h-22, "SENTINELOS", COL_GRAY, 1);
}

// ── Draw Sentinel logo ────────────────────────────────────────────────
static void draw_login_logo(int cx, int cy)
{
    // Outer diamond
    int size = 28;
    draw_line(cx, cy-size, cx+size, cy, COL_GOLD);
    draw_line(cx+size, cy, cx, cy+size, COL_GOLD);
    draw_line(cx, cy+size, cx-size, cy, COL_GOLD);
    draw_line(cx-size, cy, cx, cy-size, COL_GOLD);

    // Inner diamond
    int inner = 14;
    draw_line(cx, cy-inner, cx+inner, cy, COL_WHITE);
    draw_line(cx+inner, cy, cx, cy+inner, COL_WHITE);
    draw_line(cx, cy+inner, cx-inner, cy, COL_WHITE);
    draw_line(cx-inner, cy, cx, cy-inner, COL_WHITE);

    // Cross
    draw_line(cx-size, cy, cx+size, cy, COL_GOLD);
    draw_line(cx, cy-size, cx, cy+size, COL_GOLD);
}

// ── String helpers ────────────────────────────────────────────────────
static int lg_strlen(const char *s)
{
    int i = 0; while(s[i]) i++; return i;
}

static int lg_strcmp(const unsigned char *a, const unsigned char *b, int n)
{
    int i;
    for(i = 0; i < n; i++)
        if(a[i] != b[i]) return 0;
    return 1;
}

// ── Main login screen ─────────────────────────────────────────────────
int login_run(void)
{
    extern screen_info_t screen;
    int W = screen.width;
    int H = screen.height;
    int cx = W / 2;
    int cy = H / 2;

    // Panel dimensions
    int pw = 380;
    int ph = 420;
    int panel_top = cy - ph/2;

    // Password buffer
    char password[64];
    int  pass_len = 0;
    int  attempts = 0;
    int  error    = 0;
    int  success  = 0;

    password[0] = 0;

    // ── Initial draw ──────────────────────────────────────────────────
    draw_gradient_bg(W, H);
    draw_language_bar(W, H);

    // Clock at top
    draw_clock(W);

    // Panel
    draw_panel(cx, cy, pw, ph);

    // Logo
    draw_login_logo(cx, panel_top + 70);

    // "SentinelOS" — large title
    draw_string_scaled(
        center_x("SENTINELOS", 3, W),
        panel_top + 120,
        "SENTINELOS", COL_WHITE, 3);

    // Subtitle
    draw_string_scaled(
        center_x("SECURE OPERATING SYSTEM", 1, W),
        panel_top + 160,
        "SECURE OPERATING SYSTEM", COL_GRAY, 1);

    // Divider line
    draw_line(cx - 140, panel_top + 178, cx + 140, panel_top + 178, COL_BORDER);

    // "Welcome" text
    draw_string_scaled(
        center_x("WELCOME", 2, W),
        panel_top + 190,
        "WELCOME", COL_LIGHTGRAY, 2);

    // User name
    draw_string_scaled(
        center_x("KUNAL", 2, W),
        panel_top + 220,
        "KUNAL", COL_GOLD, 2);

    // Password label
    draw_string_scaled(
        cx - 130,
        panel_top + 258,
        "PASSWORD", COL_GRAY, 1);

    // Input box
    draw_input_box(cx, panel_top + 272, 280, 36, password, 1, 0);

    // Hint
    draw_string_scaled(
        center_x("PRESS ENTER TO LOGIN", 1, W),
        panel_top + 320,
        "PRESS ENTER TO LOGIN", COL_GRAY, 1);

    // Trust motto
    draw_string_scaled(
        center_x("TRUST NOTHING.", 1, W),
        panel_top + 345,
        "TRUST NOTHING.", COL_BORDER, 1);
    draw_string_scaled(
        center_x("VERIFY EVERYTHING.", 1, W),
        panel_top + 358,
        "VERIFY EVERYTHING.", COL_BORDER, 1);

    // ── Input loop ────────────────────────────────────────────────────
    while(!success)
    {
        char c = keyboard_poll();
        if(!c) continue;

        if(c == '\n' || c == '\r')
        {
            // Hash the entered password and compare
            unsigned char entered_hash[32];
            sha256_compute((unsigned char*)password,
                           (unsigned int)pass_len,
                           entered_hash);

                           // Show entered hash vs stored hash
 // DEBUG — show hash bytes on screen
            if(lg_strcmp(entered_hash, PASSWORD_HASH, 32))
            {
                // ── SUCCESS ───────────────────────────────────────────
                success = 1;

                // Flash green
                draw_input_box(cx, panel_top + 272, 280, 36, 0, 0, 0);
                draw_rect(cx - 140, panel_top + 272, 280, 36, COL_GREEN);
                draw_string_scaled(
                    center_x("ACCESS GRANTED", 2, W),
                    panel_top + 283,
                    "ACCESS GRANTED", 0x003300, 2);

                // Wait a moment
                volatile int delay;
                for(delay = 0; delay < 50000000; delay++);

                return LOGIN_SUCCESS;
            }
            else
            {
                // ── FAILURE ───────────────────────────────────────────
                attempts++;
                error = 1;

                // Clear input
                pass_len = 0;
                password[0] = 0;

                // Red flash
                draw_input_box(cx, panel_top + 272, 280, 36,
                               password, 1, 1);

                // Error message
                draw_rect(cx - 140, panel_top + 315, 280, 14, COL_PANEL);
                if(attempts >= 3)
                {
                    draw_string_scaled(
                        center_x("TOO MANY ATTEMPTS", 1, W),
                        panel_top + 315,
                        "TOO MANY ATTEMPTS", COL_RED, 1);
                }
                else
                {
                    draw_string_scaled(
                        center_x("INCORRECT PASSWORD", 1, W),
                        panel_top + 315,
                        "INCORRECT PASSWORD", COL_RED, 1);
                }

                // Shake animation — move panel left/right
                int shake;
                for(shake = 0; shake < 6; shake++)
                {
                    int offset = (shake % 2 == 0) ? 8 : -8;
                    draw_rect(cx - pw/2 - 10, panel_top + 265,
                              pw + 20, 50, COL_PANEL);
                    draw_input_box(cx + offset, panel_top + 272,
                                   280, 36, password, 1, 1);
                    volatile int d;
                    for(d = 0; d < 5000000; d++);
                }

                // Redraw normal input box
                draw_rect(cx - 140, panel_top + 265, 280, 50, COL_PANEL);
                draw_input_box(cx, panel_top + 272, 280, 36,
                               password, 1, 0);
                error = 0;
            }
        }
        else if(c == '\b')
        {
            // Backspace
            if(pass_len > 0)
            {
                pass_len--;
                password[pass_len] = 0;
                // Redraw input box
                draw_rect(cx - 140, panel_top + 272, 280, 36, COL_PANEL);
                draw_input_box(cx, panel_top + 272, 280, 36,
                               password, 1, 0);
            }
        }
        else if(pass_len < 63 && c >= 32 && c < 127)
        {
            // Convert uppercase to lowercase
          // Show ASCII value for debug
            // Add character
            password[pass_len++] = c;
            password[pass_len]   = 0;
            // Redraw input box with new dot
            draw_rect(cx - 140, panel_top + 272, 280, 36, COL_PANEL);
            draw_input_box(cx, panel_top + 272, 280, 36,
                           password, 1, 0);
        }
    }

    return LOGIN_SUCCESS;
}