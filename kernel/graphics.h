#ifndef GRAPHICS_H
#define GRAPHICS_H

// VGA graphics mode 13h
#define VGA_WIDTH    320
#define VGA_HEIGHT   200
#define VGA_MEMORY   0xA0000

// Basic colors (Mode 13h palette)
#define COLOR_BLACK    0
#define COLOR_BLUE     1
#define COLOR_GREEN    2
#define COLOR_CYAN     3
#define COLOR_RED      4
#define COLOR_PURPLE   5
#define COLOR_BROWN    6
#define COLOR_WHITE    15
#define COLOR_GOLD     14
#define COLOR_DARKGRAY 8

// Function signatures
void graphics_init();
void draw_pixel(int x, int y, unsigned char color);
void draw_rect(int x, int y, int w, int h, unsigned char color);
void draw_line(int x1, int y1, int x2, int y2, unsigned char color);
void clear_screen_graphics(unsigned char color);

#endif