#include "boot_anim.h"
#include "graphics.h"
#include "font.h"
#include "timer.h"

// Simple delay
// Better delay - more reliable timing
static void delay(int ms)
{
    volatile int i, j;
    for(i = 0; i < ms * 10; i++)
        for(j = 0; j < 5000; j++);
}

// Draw one pixel at a time along a line - animated
static void draw_line_anim(int x1, int y1, int x2, int y2, 
                           unsigned char color, int speed)
{
    int dx = x2-x1, dy = y2-y1;
    int steps = dx>dy ? dx : dy;
    if(steps < 0) steps = -steps;
    int i;
    for(i = 0; i <= steps; i++)
    {
        int x = x1 + (dx * i / (steps+1));
        int y = y1 + (dy * i / (steps+1));
        draw_pixel(x, y, color);
        if(i % speed == 0) delay(1);
    }
}

// Draw string letter by letter
static void draw_string_anim(int x, int y, const char *str,
                              unsigned char color, int delay_ms)
{
    int i = 0;
    char buf[2];
    buf[1] = 0;
    while(str[i])
    {
        buf[0] = str[i];
        draw_string(x + i*8, y, buf, color);
        delay(delay_ms);  // just delay_ms, not * 500
        i++;
    }
}

// Progress bar
static void draw_progress(int x, int y, int w, int percent, 
                          unsigned char color)
{
    // Background
    draw_rect(x, y, w, 6, COLOR_PANEL_BG);
    // Border
    draw_line(x,   y,   x+w, y,   COLOR_DIM_GOLD);
    draw_line(x,   y+6, x+w, y+6, COLOR_DIM_GOLD);
    draw_line(x,   y,   x,   y+6, COLOR_DIM_GOLD);
    draw_line(x+w, y,   x+w, y+6, COLOR_DIM_GOLD);
    // Fill
    int fill = (w-2) * percent / 100;
    draw_rect(x+1, y+1, fill, 4, color);
}

// Draw Sentinel logo animated
static void draw_logo_animated(int cx, int cy)
{
    // Draw outer diamond line by line with delay
    draw_line_anim(cx, cy-40, cx+30, cy,    COLOR_FORERUNNER_GOLD, 2);
    draw_line_anim(cx+30, cy, cx,    cy+40, COLOR_FORERUNNER_GOLD, 2);
    draw_line_anim(cx, cy+40, cx-30, cy,    COLOR_FORERUNNER_GOLD, 2);
    draw_line_anim(cx-30, cy, cx,    cy-40, COLOR_FORERUNNER_GOLD, 2);

    delay(200);

    // Inner diamond
    draw_line_anim(cx, cy-20, cx+15, cy,    COLOR_NEON_CYAN, 3);
    draw_line_anim(cx+15, cy, cx,    cy+20, COLOR_NEON_CYAN, 3);
    draw_line_anim(cx, cy+20, cx-15, cy,    COLOR_NEON_CYAN, 3);
    draw_line_anim(cx-15, cy, cx,    cy-20, COLOR_NEON_CYAN, 3);

    delay(100);

    // Center cross flash
    draw_line(cx-30, cy, cx+30, cy, COLOR_FORERUNNER_GOLD);
    draw_line(cx, cy-40, cx, cy+40, COLOR_FORERUNNER_GOLD);

    delay(300);
}

void boot_animation(void)
{
    clear_screen_graphics(COLOR_BLACK);
    delay(100);  // pause on black

    // PHASE 1: Logo draws itself slowly
    draw_logo_animated(160, 70);
    delay(400);

    // PHASE 2: Title letter by letter
    draw_string_anim(88, 120, "SENTINELOS", COLOR_FORERUNNER_GOLD, 15);
    delay(200);
    draw_string(96, 132, "SECURE KERNEL V0.1", COLOR_SILVER);
    delay(300);

    draw_line(60, 142, 260, 142, COLOR_DIM_GOLD);
    delay(200);

    // PHASE 3: Systems online - slower
    struct {
        const char *name;
        int delay_ms;
    } systems[] = {
        {"INTERRUPT DESCRIPTOR TABLE", 30},
        {"MEMORY MANAGER",             25},
        {"TRUST REGISTRY",             35},
        {"SHA-256 ENGINE",             40},
        {"PROCESS SCHEDULER",          28},
        {"KEYBOARD DRIVER",            20},
        {"TIMER DRIVER",               20},
        {"VERIFICATION GATE",          50},
        {"GRAPHICS ENGINE",            25},
    };

    int n = 9;
    int i;
    for(i = 0; i < n; i++)
    {
        int y = 150 + i * 10;
        draw_string(30, y, systems[i].name, COLOR_SILVER);
        delay(systems[i].delay_ms * 3);
        draw_string(200, y, ".", COLOR_DIM_GOLD);
        delay(systems[i].delay_ms * 3);
        draw_string(206, y, ".", COLOR_DIM_GOLD);
        delay(systems[i].delay_ms * 3);
        draw_string(212, y, ".", COLOR_DIM_GOLD);
        delay(systems[i].delay_ms * 3);
        draw_string(220, y, "[ OK ]", COLOR_VERIFIED_GREEN);
        draw_progress(60, 145 + n*10 + 5, 200,
                     (i+1)*100/n, COLOR_FORERUNNER_GOLD);
        delay(systems[i].delay_ms * 4);
    }

    delay(600);

    // PHASE 4: Flash
    clear_screen_graphics(COLOR_FORERUNNER_GOLD);
    delay(50);
    clear_screen_graphics(COLOR_BLACK);
    delay(50);
    clear_screen_graphics(COLOR_FORERUNNER_GOLD);
    delay(50);
    clear_screen_graphics(COLOR_BLACK);
    delay(200);

    // PHASE 5: Trust Nothing screen
    clear_screen_graphics(COLOR_SPACE_BLACK);
    draw_logo_animated(160, 80);
    delay(300);
    draw_string_anim(75, 130, "TRUST NOTHING.", COLOR_WHITE, 12);
    delay(600);
    draw_string_anim(58, 145, "VERIFY EVERYTHING.", COLOR_WHITE, 12);
    delay(1000);

    // Flash to dashboard
    clear_screen_graphics(COLOR_FORERUNNER_GOLD);
    delay(80);
}