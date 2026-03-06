#include "boot_anim.h"
#include "graphics.h"
#include "font.h"

extern screen_info_t screen;

static void delay(int ms)
{
    volatile int i, j;
    for(i = 0; i < ms * 20; i++)
        for(j = 0; j < 8000; j++);
}

// Draw glowing line (main line + dim glow around it)
static void draw_glow_line(int x1, int y1, int x2, int y2,
                           unsigned int color, unsigned int glow)
{
    // Glow layers
    draw_line(x1-1, y1, x2-1, y2, glow);
    draw_line(x1+1, y1, x2+1, y2, glow);
    draw_line(x1, y1-1, x2, y2-1, glow);
    draw_line(x1, y1+1, x2, y2+1, glow);
    // Main line
    draw_line(x1, y1, x2, y2, color);
}

// Animated line pixel by pixel
static void draw_line_anim(int x1, int y1, int x2, int y2,
                           unsigned int color, unsigned int glow, int spd)
{
    int dx = x2-x1, dy = y2-y1;
    int steps = dx > dy ? dx : dy;
    if(steps < 0) steps = -steps;
    if(steps == 0) return;
    int i;
    for(i = 0; i <= steps; i++)
    {
        int x = x1 + (dx * i / steps);
        int y = y1 + (dy * i / steps);
        // Glow
        draw_pixel(x-1, y,   glow);
        draw_pixel(x+1, y,   glow);
        draw_pixel(x,   y-1, glow);
        draw_pixel(x,   y+1, glow);
        // Main
        draw_pixel(x, y, color);
        if(i % spd == 0) delay(1);
    }
}

// Draw letter by letter animated
static void draw_text_anim(int x, int y, const char *str,
                           unsigned int color, int ms)
{
    int i = 0;
    char buf[2]; buf[1] = 0;
    while(str[i])
    {
        buf[0] = str[i];
        draw_string(x + i*10, y, buf, color);
        // Glow effect - redraw slightly offset in dim color
        unsigned int glow = 0x331A00;
        if(color == 0xFFC832) glow = 0x332800;
        if(color == 0x00FFFF) glow = 0x003333;
        if(color == 0xFFFFFF) glow = 0x222222;
        draw_string(x + i*10 - 1, y, buf, glow);
        delay(ms);
        i++;
    }
}

// Draw centered text
static void draw_centered(int y, const char *str, unsigned int color)
{
    int len = 0;
    while(str[len]) len++;
    int x = (screen.width - len * 8) / 2;
    draw_string(x, y, str, color);
}

// Draw centered text animated
static void draw_centered_anim(int y, const char *str,
                               unsigned int color, int ms)
{
    int len = 0;
    while(str[len]) len++;
    int x = (screen.width - len * 10) / 2;
    draw_text_anim(x, y, str, color, ms);
}

// Draw big diamond logo centered and animated
static void draw_diamond(int cx, int cy, int size, int animate)
{
    unsigned int gold     = 0xFFC832;
    unsigned int cyan     = 0x00FFFF;
    unsigned int dgold    = 0x664E14;  // dim gold glow
    unsigned int dcyan    = 0x004444;  // dim cyan glow

    int ox = size;
    int oy = size * 3 / 2;
    int ix = size / 2;
    int iy = size * 3 / 4;

    if(animate)
    {
        // Outer diamond - draw each side animated
        draw_line_anim(cx,    cy-oy, cx+ox, cy,    gold, dgold, 3);
        draw_line_anim(cx+ox, cy,    cx,    cy+oy, gold, dgold, 3);
        draw_line_anim(cx,    cy+oy, cx-ox, cy,    gold, dgold, 3);
        draw_line_anim(cx-ox, cy,    cx,    cy-oy, gold, dgold, 3);
        delay(150);

        // Inner diamond
        draw_line_anim(cx,    cy-iy, cx+ix, cy,    cyan, dcyan, 4);
        draw_line_anim(cx+ix, cy,    cx,    cy+iy, cyan, dcyan, 4);
        draw_line_anim(cx,    cy+iy, cx-ix, cy,    cyan, dcyan, 4);
        draw_line_anim(cx-ix, cy,    cx,    cy-iy, cyan, dcyan, 4);
        delay(100);

        // Center cross
        draw_glow_line(cx-ox, cy, cx+ox, cy, gold, dgold);
        draw_glow_line(cx, cy-oy, cx, cy+oy, gold, dgold);
    }
    else
    {
        draw_glow_line(cx,    cy-oy, cx+ox, cy,    gold, dgold);
        draw_glow_line(cx+ox, cy,    cx,    cy+oy, gold, dgold);
        draw_glow_line(cx,    cy+oy, cx-ox, cy,    gold, dgold);
        draw_glow_line(cx-ox, cy,    cx,    cy-oy, gold, dgold);
        draw_glow_line(cx,    cy-iy, cx+ix, cy,    cyan, dcyan);
        draw_glow_line(cx+ix, cy,    cx,    cy+iy, cyan, dcyan);
        draw_glow_line(cx,    cy+iy, cx-ix, cy,    cyan, dcyan);
        draw_glow_line(cx-ix, cy,    cx,    cy-iy, cyan, dcyan);
        draw_glow_line(cx-ox, cy, cx+ox, cy, gold, dgold);
        draw_glow_line(cx, cy-oy, cx, cy+oy, gold, dgold);
    }
}

// Scanline effect
static void draw_scanlines()
{
    int i;
    for(i = 0; i < screen.height; i += 4)
        draw_line(0, i, screen.width, i, 0x03050F);
}

// Progress bar centered
static void draw_progress_bar(int percent, unsigned int color)
{
    int w = screen.width / 2;
    int x = screen.width / 4;
    int y = screen.height * 3 / 4 + 40;
    int h = 4;

    draw_rect(x-1, y-1, w+2, h+2, 0x111111);
    draw_line(x, y, x+w, y, 0x333333);
    int fill = w * percent / 100;
    draw_rect(x, y, fill, h, color);

    // Glow tip
    if(fill > 2)
    {
        draw_rect(x+fill-2, y-1, 4, h+2, 0xFFFFAA);
    }
}

// Flash effect
static void flash(unsigned int color, int ms)
{
    clear_screen_graphics(color);
    delay(ms);
    clear_screen_graphics(0x000000);
    delay(ms);
}

void boot_animation(void)
{
    int cx = screen.width  / 2;
    int cy = screen.height / 2;

    // ── PHASE 1: Black screen fade in ─────────────────
    clear_screen_graphics(0x000000);
    delay(400);

    // ── PHASE 2: Scanlines appear ──────────────────────
    draw_scanlines();
    delay(100);

    // ── PHASE 3: Diamond draws itself ─────────────────
    draw_diamond(cx, cy - 80, 60, 1);
    delay(500);

    // ── PHASE 4: SENTINELOS title ─────────────────────
    draw_centered_anim(cy + 10, "SENTINELOS", 0xFFC832, 8);
    delay(400);

    // Subtitle
    draw_centered(cy + 30, "SECURE  KERNEL  V0.1", 0x888888);
    delay(200);

    // Divider line
    int lx = screen.width / 4;
    draw_glow_line(lx, cy+45, screen.width*3/4, cy+45, 0xFFC832, 0x332800);
    delay(300);

    // ── PHASE 5: Systems loading ───────────────────────
    struct { const char *name; int ms; } sys[] = {
        {"MEMORY MANAGER",    20},
        {"TRUST REGISTRY",    25},
        {"SHA-256 ENGINE",    30},
        {"VERIFICATION GATE", 35},
        {"GRAPHICS ENGINE",   20},
    };

    int n = 5;
    int i;
    int sy = cy + 60;
    for(i = 0; i < n; i++)
    {
        int lw = 0;
        while(sys[i].name[lw]) lw++;
        int tx = (screen.width - lw*8 - 80) / 2;

        draw_string(tx, sy + i*16, sys[i].name, 0x888888);
        delay(sys[i].ms * 4);
        draw_string(tx, sy + i*16, sys[i].name, 0xCCCCCC);
        draw_string(tx + lw*8 + 10, sy + i*16, "[ OK ]", 0x00FF64);

        draw_progress_bar((i+1)*100/n, 0xFFC832);
        delay(sys[i].ms * 5);
    }

    delay(800);

    // ── PHASE 6: TRUST NOTHING flash ──────────────────
    clear_screen_graphics(0x000000);
    delay(100);

    draw_scanlines();
    draw_diamond(cx, cy - 60, 50, 0);
    delay(200);

    draw_centered_anim(cy + 20,  "TRUST  NOTHING.", 0xFFFFFF, 6);
    delay(400);
    draw_centered_anim(cy + 40, "VERIFY  EVERYTHING.", 0xFFC832, 6);
    delay(800);

    // ── PHASE 7: Final flash to dashboard ─────────────
    flash(0xFFC832, 40);
    delay(50);
    flash(0x00FFFF, 30);
    delay(50);
    clear_screen_graphics(0x000000);
    delay(100);
}