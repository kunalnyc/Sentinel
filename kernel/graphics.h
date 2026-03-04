#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>



// Default resolution - change these to your preference
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768
#define SCREEN_BPP      32     // 32-bit true color

// Forerunner palette in 32-bit RGB format
#define COLOR_BLACK         0x00000000
#define COLOR_SPACE_BLACK   0x050514
#define COLOR_FORERUNNER_GOLD 0xFFC832
#define COLOR_HARD_LIGHT    0x0096FF
#define COLOR_NEON_CYAN     0x00FFDC
#define COLOR_VERIFIED_GREEN 0x00FF64
#define COLOR_ALERT_RED     0xFF1E1E
#define COLOR_DEEP_BLUE     0x0A0F28
#define COLOR_WHITE         0xFFFFFF
#define COLOR_SILVER        0xB4B4C8
#define COLOR_DIM_GOLD      0x96781E
#define COLOR_PANEL_BG      0x080C23
#define COLOR_SCAN_LINE     0x03050F

// Screen info structure
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t framebuffer;      // physical address
    uint32_t framebuffer_virtual; // virtual address after mapping
    uint32_t bytes_per_pixel;
} screen_info_t;

// Function signatures
int graphics_init(uint32_t width, uint32_t height, uint32_t bpp);
void graphics_init_vga(void); // Fallback to 13h if VBE fails
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void clear_screen_graphics(uint32_t color);
void bochs_vbe_init(uint16_t width, uint16_t height, uint16_t bpp);
void set_palette_color(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
void init_forerunner_palette(void);

// New high-resolution functions
void draw_char(char c, int x, int y, uint32_t color, uint32_t bgcolor);
void draw_string(int x, int y, const char* str, uint32_t color);
void draw_string_bg(int x, int y, const char* str, uint32_t color, uint32_t bgcolor);
void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color);
void draw_gradient_rect(int x, int y, int w, int h, uint32_t color1, uint32_t color2, int vertical);
void draw_alpha_blend(int x, int y, int w, int h, uint32_t color, uint8_t alpha);

extern screen_info_t screen;

#endif