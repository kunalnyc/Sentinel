// Sentinel kernel main file
// Entry point - first code the CPU runs after bootloader
#include "../security/trust.h"
#include "../security/sha256.h"
#include "idt.h"
#include "memory.h"
#include "scheduler.h"
#include "keyboard.h"
#include "timer.h"
#include "graphics.h"
#include "font.h"
// Actual definitions live here
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

int cursor_row = 0;
int cursor_col = 0;

void print(char *str)
{
    char *video = (char *)0xB8000;
    int i = 0;

    while(str[i] != '\0')
    {
        int pos = (cursor_row * 80 + cursor_col) * 2;
        video[pos]     = str[i];
        video[pos + 1] = 0x0F;
        cursor_col++;
        i++;
    }
}
void println(char *str)
{
    print(str);
    cursor_row++;
    cursor_col = 0;
}

void clear_screen() {
    char *video = (char *)0xB8000; // VGA video memory starts at 0xB8000
    for (int i = 0; i < 80 * 25; i++) {
        video[i * 2] = ' '; // Clear character
        video[i * 2 + 1] = 0x0F; // Attribute byte (white on black)
    }
}


// Forward declarations
void println(char *str);
void print(char *str);

// Draw the Sentinel geometric logo
void draw_sentinel_logo(int cx, int cy)
{
    // Outer diamond
    draw_line(cx,      cy-40, cx+30, cy,    COLOR_GOLD);
    draw_line(cx+30,   cy,    cx,    cy+40, COLOR_GOLD);
    draw_line(cx,      cy+40, cx-30, cy,    COLOR_GOLD);
    draw_line(cx-30,   cy,    cx,    cy-40, COLOR_GOLD);

    // Inner diamond
    draw_line(cx,      cy-20, cx+15, cy,    COLOR_WHITE);
    draw_line(cx+15,   cy,    cx,    cy+20, COLOR_WHITE);
    draw_line(cx,      cy+20, cx-15, cy,    COLOR_WHITE);
    draw_line(cx-15,   cy,    cx,    cy-20, COLOR_WHITE);

    // Center cross
    draw_line(cx-30, cy, cx+30, cy, COLOR_GOLD);
    draw_line(cx, cy-40, cx, cy+40, COLOR_GOLD);

    // Corner accents
    draw_line(cx-30, cy-10, cx-20, cy-10, COLOR_GOLD);
    draw_line(cx+20, cy-10, cx+30, cy-10, COLOR_GOLD);
    draw_line(cx-30, cy+10, cx-20, cy+10, COLOR_GOLD);
    draw_line(cx+20, cy+10, cx+30, cy+10, COLOR_GOLD);
}

void boot_animation()
{
    clear_screen_graphics(COLOR_BLACK);

    // Draw logo piece by piece with delays
    int cx = 160;
    int cy = 80;

    // Outer diamond draws itself
    draw_line(cx, cy-40, cx+30, cy, COLOR_GOLD);
    timer_wait(50);
    draw_line(cx+30, cy, cx, cy+40, COLOR_GOLD);
    timer_wait(50);
    draw_line(cx, cy+40, cx-30, cy, COLOR_GOLD);
    timer_wait(50);
    draw_line(cx-30, cy, cx, cy-40, COLOR_GOLD);
    timer_wait(50);

    // Inner diamond
    draw_line(cx,    cy-20, cx+15, cy,    COLOR_WHITE);
    draw_line(cx+15, cy,    cx,    cy+20, COLOR_WHITE);
    draw_line(cx,    cy+20, cx-15, cy,    COLOR_WHITE);
    draw_line(cx-15, cy,    cx,    cy-20, COLOR_WHITE);
    timer_wait(50);

    // Center cross
    draw_line(cx-30, cy, cx+30, cy, COLOR_GOLD);
    draw_line(cx, cy-40, cx, cy+40, COLOR_GOLD);
    timer_wait(50);

    // Title appears
    draw_string(100, 140, "SENTINELOS", COLOR_GOLD);
    timer_wait(50);
    draw_string(75, 155, "TRUST NOTHING.", COLOR_WHITE);
    timer_wait(50);
    draw_string(75, 165, "VERIFY EVERYTHING.", COLOR_WHITE);
    timer_wait(50);
}

void draw_main_screen()
{
    // Deep space background
    clear_screen_graphics(COLOR_SPACE_BLACK);

    // Scan lines effect - alternating dark lines
    int i;
    for(i = 0; i < 200; i += 2)
        draw_line(0, i, 320, i, COLOR_SCAN_LINE);

    // Top header bar
    draw_rect(0, 0, 320, 12, COLOR_PANEL_BG);
    draw_line(0, 12, 320, 12, COLOR_FORERUNNER_GOLD);
    draw_string(2, 2, "SENTINELOS", COLOR_FORERUNNER_GOLD);
    draw_string(220, 2, "SECURE KERNEL V0.1", COLOR_SILVER);

    // Bottom status bar
    draw_rect(0, 188, 320, 12, COLOR_PANEL_BG);
    draw_line(0, 188, 320, 188, COLOR_FORERUNNER_GOLD);
    draw_string(2, 191, "ALL SYSTEMS OPERATIONAL", COLOR_VERIFIED_GREEN);
    draw_string(240, 191, "100HZ TIMER", COLOR_SILVER);

    // Left panel - Security Status
    draw_rect(5, 18, 145, 165, COLOR_PANEL_BG);
    draw_line(5,   18, 150, 18,  COLOR_FORERUNNER_GOLD);
    draw_line(5,   183, 150, 183, COLOR_FORERUNNER_GOLD);
    draw_line(5,   18, 5,   183, COLOR_FORERUNNER_GOLD);
    draw_line(150, 18, 150, 183, COLOR_FORERUNNER_GOLD);

    // Left panel title
    draw_line(5, 28, 150, 28, COLOR_DIM_GOLD);
    draw_string(10, 20, "SECURITY STATUS", COLOR_FORERUNNER_GOLD);

    // Security items
    draw_string(10, 35,  "TRUST REGISTRY", COLOR_SILVER);
    draw_string(10, 45,  "VERIFICATION GATE", COLOR_SILVER);
    draw_string(10, 55,  "SHA-256 ENGINE", COLOR_SILVER);
    draw_string(10, 65,  "MEMORY GUARD", COLOR_SILVER);
    draw_string(10, 75,  "PROCESS SHIELD", COLOR_SILVER);
    draw_string(10, 85,  "IDT PROTECTION", COLOR_SILVER);

    // Status indicators
    draw_string(110, 35, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 45, "ACTIVE", COLOR_VERIFIED_GREEN);
    draw_string(110, 55, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 65, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(110, 75, "ACTIVE", COLOR_VERIFIED_GREEN);
    draw_string(110, 85, "ONLINE", COLOR_VERIFIED_GREEN);

    // Threat counter
    draw_line(5, 100, 150, 100, COLOR_DIM_GOLD);
    draw_string(10, 103, "THREATS BLOCKED", COLOR_FORERUNNER_GOLD);
    draw_string(10, 115, "TODAY", COLOR_SILVER);
    draw_string(80, 113, "000001", COLOR_ALERT_RED);
    draw_string(10, 128, "TOTAL", COLOR_SILVER);
    draw_string(80, 128, "000001", COLOR_ALERT_RED);

    // Right panel - System Info
    draw_rect(155, 18, 160, 165, COLOR_PANEL_BG);
    draw_line(155, 18,  315, 18,  COLOR_FORERUNNER_GOLD);
    draw_line(155, 183, 315, 183, COLOR_FORERUNNER_GOLD);
    draw_line(155, 18,  155, 183, COLOR_FORERUNNER_GOLD);
    draw_line(315, 18,  315, 183, COLOR_FORERUNNER_GOLD);

    // Right panel title
    draw_line(155, 28, 315, 28, COLOR_DIM_GOLD);
    draw_string(160, 20, "SYSTEM MONITOR", COLOR_FORERUNNER_GOLD);

    // Sentinel logo
    draw_sentinel_logo(235, 80);

    // System info below logo
    draw_line(155, 128, 315, 128, COLOR_DIM_GOLD);
    draw_string(160, 132, "KERNEL", COLOR_SILVER);
    draw_string(220, 132, "0x100000", COLOR_HARD_LIGHT);
    draw_string(160, 142, "MEMORY", COLOR_SILVER);
    draw_string(220, 142, "16MB", COLOR_HARD_LIGHT);
    draw_string(160, 152, "PAGES", COLOR_SILVER);
    draw_string(220, 152, "4096", COLOR_HARD_LIGHT);
    draw_string(160, 162, "TIMER", COLOR_SILVER);
    draw_string(220, 162, "100HZ", COLOR_HARD_LIGHT);
    draw_string(160, 172, "ARCH", COLOR_SILVER);
    draw_string(220, 172, "X86-32", COLOR_HARD_LIGHT);
}
void kernel_main(void)
{
    // Initialize systems FIRST
    idt_init();
    memory_init();
    timer_init();      // timer before animation!
    keyboard_init();

    // THEN switch to graphics
    graphics_init();

    // Now animation will work properly
   // boot_animation();
    draw_main_screen();

    while(1) {}
}