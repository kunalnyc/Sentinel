#include "../security/trust.h"
#include "../security/sha256.h"
#include "idt.h"
#include "memory.h"
#include "scheduler.h"
#include "keyboard.h"
#include "timer.h"
#include "graphics.h"
#include "font.h"
// IDT definitions
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

// Multiboot2 structures
struct MB2Tag {
    unsigned int type;
    unsigned int size;
};

struct MB2Framebuffer {
    unsigned int type;
    unsigned int size;
    unsigned long long addr;
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
    unsigned char bpp;
    unsigned char fb_type;
    unsigned short reserved;
};

struct MB2Header {
    unsigned int total_size;
    unsigned int reserved;
};

// Global framebuffer
unsigned long long fb_addr  = 0;
unsigned int fb_width  = 0;
unsigned int fb_height = 0;
unsigned int fb_pitch  = 0;

// Parse multiboot2 info
void parse_multiboot(unsigned int mb_addr)
{
    unsigned long long addr = (unsigned long long)mb_addr;
    unsigned int total = ((struct MB2Header*)addr)->total_size;
    unsigned long long end = addr + total;
    addr += 8;

    while(addr < end)
    {
        struct MB2Tag *tag = (struct MB2Tag*)addr;
        if(tag->type == 0) break;

        if(tag->type == 8)
        {
            struct MB2Framebuffer *fb = (struct MB2Framebuffer*)addr;
            fb_addr   = fb->addr;
            fb_width  = fb->width;
            fb_height = fb->height;
            fb_pitch  = fb->pitch;
        }
        addr += (tag->size + 7) & ~7;
    }
}

// Framebuffer drawing functions
void put_pixel(int x, int y, unsigned int color)
{
    if(!fb_addr) return;
    if(x < 0 || x >= (int)fb_width)  return;
    if(y < 0 || y >= (int)fb_height) return;
    unsigned int *fb = (unsigned int*)fb_addr;
    fb[y * (fb_pitch/4) + x] = color;
}

void fill_rect(int x, int y, int w, int h, unsigned int color)
{
    int i, j;
    for(j = y; j < y+h; j++)
        for(i = x; i < x+w; i++)
            put_pixel(i, j, color);
}

void draw_line_fb(int x1, int y1, int x2, int y2, unsigned int color)
{
    int dx = x2-x1, dy = y2-y1;
    int steps = dx>dy ? dx : dy;
    if(steps < 0) steps = -steps;
    if(steps == 0) { put_pixel(x1, y1, color); return; }
    int i;
    for(i = 0; i <= steps; i++)
    {
        int x = x1 + (dx * i / steps);
        int y = y1 + (dy * i / steps);
        put_pixel(x, y, color);
    }
}

// Forerunner color palette
#define BLACK   0x00050514
#define GOLD    0x00FFC832
#define GREEN   0x0000FF64
#define CYAN    0x0000FFDC
#define WHITE   0x00FFFFFF
#define PANEL   0x0008080F
#define DGOLD   0x00886814
#define RED     0x00FF1E1E
#define SILVER  0x00B4B4C8

void draw_main_screen()
{
    // Deep space background
    fill_rect(0, 0, fb_width, fb_height, BLACK);

    // Scanlines
    unsigned int i;
    for(i = 0; i < fb_height; i += 3)
        draw_line_fb(0, i, fb_width, i, 0x00030308);

    // Top bar
    fill_rect(0, 0, fb_width, 20, PANEL);
    draw_line_fb(0, 20, fb_width, 20, GOLD);
    draw_line_fb(0, 22, fb_width, 22, DGOLD);

    // Bottom bar
    fill_rect(0, fb_height-20, fb_width, 20, PANEL);
    draw_line_fb(0, fb_height-20, fb_width, fb_height-20, GOLD);

    // Left panel
    fill_rect(10, 30, (fb_width/2)-20, fb_height-60, PANEL);
    draw_line_fb(10, 30, (fb_width/2)-10, 30, GOLD);
    draw_line_fb(10, fb_height-30, (fb_width/2)-10, fb_height-30, GOLD);
    draw_line_fb(10, 30, 10, fb_height-30, GOLD);
    draw_line_fb((fb_width/2)-10, 30, (fb_width/2)-10, fb_height-30, GOLD);

    // Right panel
    fill_rect(fb_width/2+10, 30, (fb_width/2)-20, fb_height-60, PANEL);
    draw_line_fb(fb_width/2+10, 30, fb_width-10, 30, GOLD);
    draw_line_fb(fb_width/2+10, fb_height-30, fb_width-10, fb_height-30, GOLD);
    draw_line_fb(fb_width/2+10, 30, fb_width/2+10, fb_height-30, GOLD);
    draw_line_fb(fb_width-10, 30, fb_width-10, fb_height-30, GOLD);

    // Corner accents left panel
    fill_rect(10, 30, 30, 3, GOLD);
    fill_rect((fb_width/2)-40, 30, 30, 3, GOLD);
    fill_rect(10, fb_height-32, 30, 3, GOLD);
    fill_rect((fb_width/2)-40, fb_height-32, 30, 3, GOLD);

    // Corner accents right panel
    fill_rect(fb_width/2+10, 30, 30, 3, GOLD);
    fill_rect(fb_width-40, 30, 30, 3, GOLD);
    fill_rect(fb_width/2+10, fb_height-32, 30, 3, GOLD);
    fill_rect(fb_width-40, fb_height-32, 30, 3, GOLD);

    // Sentinel diamond logo center of right panel
    int cx = fb_width*3/4;
    int cy = fb_height/2 - 20;
    draw_line_fb(cx, cy-60, cx+45, cy, GOLD);
    draw_line_fb(cx+45, cy, cx, cy+60, GOLD);
    draw_line_fb(cx, cy+60, cx-45, cy, GOLD);
    draw_line_fb(cx-45, cy, cx, cy-60, GOLD);
    draw_line_fb(cx, cy-30, cx+22, cy, CYAN);
    draw_line_fb(cx+22, cy, cx, cy+30, CYAN);
    draw_line_fb(cx, cy+30, cx-22, cy, CYAN);
    draw_line_fb(cx-22, cy, cx, cy-30, CYAN);
    draw_line_fb(cx-45, cy, cx+45, cy, GOLD);
    draw_line_fb(cx, cy-60, cx, cy+60, GOLD);
}
void draw_sentinel_logo(int cx, int cy)
{
    // Outer diamond
    draw_line(cx,      cy-40, cx+30, cy,    COLOR_FORERUNNER_GOLD);
    draw_line(cx+30,   cy,    cx,    cy+40, COLOR_FORERUNNER_GOLD);
    draw_line(cx,      cy+40, cx-30, cy,    COLOR_FORERUNNER_GOLD);
    draw_line(cx-30,   cy,    cx,    cy-40, COLOR_FORERUNNER_GOLD);

    // Inner diamond
    draw_line(cx,      cy-20, cx+15, cy,    COLOR_NEON_CYAN);
    draw_line(cx+15,   cy,    cx,    cy+20, COLOR_NEON_CYAN);
    draw_line(cx,      cy+20, cx-15, cy,    COLOR_NEON_CYAN);
    draw_line(cx-15,   cy,    cx,    cy-20, COLOR_NEON_CYAN);

    // Center cross
    draw_line(cx-30, cy, cx+30, cy, COLOR_FORERUNNER_GOLD);
    draw_line(cx, cy-40, cx, cy+40, COLOR_FORERUNNER_GOLD);

    // Corner accents
    draw_line(cx-30, cy-10, cx-20, cy-10, COLOR_FORERUNNER_GOLD);
    draw_line(cx+20, cy-10, cx+30, cy-10, COLOR_FORERUNNER_GOLD);
    draw_line(cx-30, cy+10, cx-20, cy+10, COLOR_FORERUNNER_GOLD);
    draw_line(cx+20, cy+10, cx+30, cy+10, COLOR_FORERUNNER_GOLD);
}
void kernel_main(unsigned int magic, unsigned int mb_addr)
{
    // Initialize our VGA graphics mode directly
    // This programs VGA hardware registers for Mode 13h
    graphics_init();
    init_forerunner_palette();
    
    // Now draw our screen
    clear_screen_graphics(COLOR_SPACE_BLACK);
    
    // Scan lines
    int i;
    for(i = 0; i < 200; i += 2)
        draw_line(0, i, 320, i, COLOR_SCAN_LINE);

    // Top bar
    draw_rect(0, 0, 320, 10, COLOR_PANEL_BG);
    draw_line(0, 10, 320, 10, COLOR_FORERUNNER_GOLD);

    // Bottom bar  
    draw_rect(0, 190, 320, 10, COLOR_PANEL_BG);
    draw_line(0, 190, 320, 190, COLOR_FORERUNNER_GOLD);

    // Main panel
    draw_rect(5, 15, 310, 170, COLOR_PANEL_BG);
    draw_line(5, 15, 315, 15, COLOR_FORERUNNER_GOLD);
    draw_line(5, 185, 315, 185, COLOR_FORERUNNER_GOLD);
    draw_line(5, 15, 5, 185, COLOR_FORERUNNER_GOLD);
    draw_line(315, 15, 315, 185, COLOR_FORERUNNER_GOLD);

    // Logo
    draw_sentinel_logo(240, 90);

    // Text using our font engine
    draw_string(10, 20, "SENTINELOS 64-BIT", COLOR_FORERUNNER_GOLD);
    draw_string(10, 35, "TRUST REGISTRY", COLOR_SILVER);
    draw_string(150, 35, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(10, 45, "SHA-256 ENGINE", COLOR_SILVER);
    draw_string(150, 45, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(10, 55, "MEMORY GUARD", COLOR_SILVER);
    draw_string(150, 55, "ONLINE", COLOR_VERIFIED_GREEN);
    draw_string(10, 65, "VERIFY GATE", COLOR_SILVER);
    draw_string(150, 65, "ACTIVE", COLOR_FORERUNNER_GOLD);
    draw_string(10, 175, "ALL SYSTEMS OPERATIONAL", COLOR_VERIFIED_GREEN);

    while(1) {}
}