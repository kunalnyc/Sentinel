#ifndef GRAPHICS_H
#define GRAPHICS_H

// VGA graphics mode 13h
#define VGA_WIDTH    320
#define VGA_HEIGHT   200
#define VGA_MEMORY   0xA0000

// Basic colors (Mode 13h palette)

#define COLOR_BLUE     1
#define COLOR_GREEN    2
#define COLOR_CYAN     3
#define COLOR_RED      4
#define COLOR_PURPLE   5
#define COLOR_BROWN    6
#define COLOR_GOLD     14
#define COLOR_DARKGRAY 8

// Custom Forerunner palette indices
#define COLOR_BLACK         0
#define COLOR_SPACE_BLACK   1
#define COLOR_FORERUNNER_GOLD 2
#define COLOR_HARD_LIGHT    3
#define COLOR_NEON_CYAN     4
#define COLOR_VERIFIED_GREEN 5
#define COLOR_ALERT_RED     6
#define COLOR_DEEP_BLUE     7
#define COLOR_WHITE         8
#define COLOR_SILVER        9
#define COLOR_DIM_GOLD      10
#define COLOR_PANEL_BG      11
#define COLOR_SCAN_LINE     12

// Function signatures
void graphics_init();
void draw_pixel(int x, int y, unsigned char color);
void draw_rect(int x, int y, int w, int h, unsigned char color);
void draw_line(int x1, int y1, int x2, int y2, unsigned char color);
void clear_screen_graphics(unsigned char color);
void bochs_set_mode(unsigned short width, unsigned short height, unsigned short bpp);
void set_palette_color(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
void init_forerunner_palette();

#endif