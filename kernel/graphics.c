#include "graphics.h"
#include "font.h"
#include "io.h"           // Replace all inline port functions with this
#include <stdint.h>
#include <stddef.h>       // for uintptr_t

// Bochs VBE Display Interface
#define VBE_DISPI_IOPORT_INDEX  0x01CE
#define VBE_DISPI_IOPORT_DATA   0x01CF

#define VBE_DISPI_INDEX_ID          0
#define VBE_DISPI_INDEX_XRES        1
#define VBE_DISPI_INDEX_YRES        2
#define VBE_DISPI_INDEX_BPP         3
#define VBE_DISPI_INDEX_ENABLE      4
#define VBE_DISPI_INDEX_BANK        5
#define VBE_DISPI_INDEX_VIRT_WIDTH  6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET    8
#define VBE_DISPI_INDEX_Y_OFFSET    9

#define VBE_DISPI_DISABLED      0x00
#define VBE_DISPI_ENABLED       0x01
#define VBE_DISPI_LFB_ENABLED   0x40

// Global screen info
screen_info_t screen;

// VBE functions - now using io.h functions
static void vbe_write(uint16_t index, uint16_t data) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, data);
}

uint16_t vbe_read_id(void) {
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ID);
    return inw(VBE_DISPI_IOPORT_DATA);
}

void bochs_vbe_init(uint16_t width, uint16_t height, uint16_t bpp) {
    // Disable first
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    
    // Set mode parameters
    vbe_write(VBE_DISPI_INDEX_XRES, width);
    vbe_write(VBE_DISPI_INDEX_YRES, height);
    vbe_write(VBE_DISPI_INDEX_BPP, bpp);
    vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, width);
    
    // Enable with LFB
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    
    // Update screen info
    screen.width = width;
    screen.height = height;
    screen.bpp = bpp;
    screen.bytes_per_pixel = (bpp + 7) / 8;
    screen.pitch = width * screen.bytes_per_pixel;
    
    // Try different framebuffer addresses
    // Common LFB addresses: 0xE0000000, 0xFD000000, 0xF0000000
    screen.framebuffer = 0xFD000000;  // Try this address instead
    screen.framebuffer_virtual = screen.framebuffer;
    
    // Debug output
    outb(0xE9, 'F');
    outb(0xE9, 'B');
    outb(0xE9, ':');
    // Simple hex output would go here
}

// VGA Mode 13h fallback - now using io.h functions
static void vga_mode_13h_init(void) {
    // Miscellaneous output register
    outb(0x3C2, 0x63);

    // Sequencer registers
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);
    outb(0x3C4, 0x01); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F);
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E);

    // Unlock CRTC registers
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & 0x7F);

    // CRTC registers
    unsigned char crtc[] = {
        0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
        0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
        0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,0xFF
    };
    for(int i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }

    // Graphics controller registers
    unsigned char gc[] = {
        0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF
    };
    for(int i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, gc[i]);
    }

    // Attribute controller registers
    inb(0x3DA);
    unsigned char ac[] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x41,0x00,0x0F,0x00,0x00
    };
    for(int i = 0; i < 21; i++) {
        outb(0x3C0, i);
        outb(0x3C0, ac[i]);
    }
    outb(0x3C0, 0x20);
    
    screen.width = 320;
    screen.height = 200;
    screen.bpp = 8;
    screen.bytes_per_pixel = 1;
    screen.pitch = 320;
    screen.framebuffer = 0xA0000;
    screen.framebuffer_virtual = screen.framebuffer;
}

void graphics_init_vga(void) {
    vga_mode_13h_init();
}

int graphics_init(uint32_t width, uint32_t height, uint32_t bpp) {
    // Try Bochs VBE first
    uint16_t vbe_id = vbe_read_id();
    
    if (vbe_id == 0xB0C5 || vbe_id == 0xB0C4) {
        // Bochs VBE available
        bochs_vbe_init(width, height, bpp);
        return 1; // Success with high resolution
    } else {
        // Fallback to VGA Mode 13h
        vga_mode_13h_init();
        return 0; // Fallback mode
    }
}

// 32-bit pixel drawing
void draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)screen.width || y < 0 || y >= (int)screen.height)
        return;
    
    if (screen.bpp == 32) {
        uint32_t* fb = (uint32_t*)(uintptr_t)screen.framebuffer_virtual;
        fb[y * (screen.pitch/4) + x] = color;
    } else if (screen.bpp == 8) {
        uint8_t* fb8 = (uint8_t*)(uintptr_t)screen.framebuffer_virtual;
        fb8[y * screen.pitch + x] = (uint8_t)color;
    }
}

void clear_screen_graphics(uint32_t color) {
    uint32_t pixels = screen.width * screen.height;
    
    if (screen.bpp == 32) {
        uint32_t* fb = (uint32_t*)(uintptr_t)screen.framebuffer_virtual;
        for (uint32_t i = 0; i < pixels; i++) {
            fb[i] = color;
        }
    } else if (screen.bpp == 8) {
        uint8_t* fb8 = (uint8_t*)(uintptr_t)screen.framebuffer_virtual;
        for (uint32_t i = 0; i < pixels; i++) {
            fb8[i] = (uint8_t)color;
        }
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            draw_pixel(i, j, color);
        }
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy ? dx : dy);
    if (steps < 0) steps = -steps;
    if (steps == 0) {
        draw_pixel(x1, y1, color);
        return;
    }
    
    for (int i = 0; i <= steps; i++) {
        int x = x1 + (dx * i / steps);
        int y = y1 + (dy * i / steps);
        draw_pixel(x, y, color);
    }
}

void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color) {
    if (radius < 0) radius = 0;
    if (radius > w/2) radius = w/2;
    if (radius > h/2) radius = h/2;
    
    // Draw main rectangle
    draw_rect(x + radius, y, w - 2*radius, h, color);
    draw_rect(x, y + radius, w, h - 2*radius, color);
    
    // Draw corners (simple approximation)
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            if (i*i + j*j <= radius*radius) {
                draw_pixel(x + radius + i, y + radius + j, color);
                draw_pixel(x + w - radius - 1 + i, y + radius + j, color);
                draw_pixel(x + radius + i, y + h - radius - 1 + j, color);
                draw_pixel(x + w - radius - 1 + i, y + h - radius - 1 + j, color);
            }
        }
    }
}

void set_palette_color(unsigned char index, unsigned char r, unsigned char g, unsigned char b) {
    outb(0x3C8, index);
    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}

void init_forerunner_palette(void) {
    set_palette_color(0,  0,   0,   0);
    set_palette_color(1,  5,   5,   20);
    set_palette_color(2,  255, 200, 50);
    set_palette_color(3,  0,   150, 255);
    set_palette_color(4,  0,   255, 220);
    set_palette_color(5,  0,   255, 100);
    set_palette_color(6,  255, 30,  30);
    set_palette_color(7,  10,  15,  40);
    set_palette_color(8,  255, 255, 255);
    set_palette_color(9,  180, 180, 200);
    set_palette_color(10, 150, 120, 30);
    set_palette_color(11, 8,   12,  35);
    set_palette_color(12, 3,   5,   15);
}