#include "graphics.h"

// Write byte to hardware port
static void port_write(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Read byte from hardware port
static unsigned char port_read(unsigned short port)
{
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void graphics_init()
{
    // Program VGA registers directly for Mode 13h
    // Miscellaneous output register
    port_write(0x3C2, 0x63);

    // Sequencer registers
    port_write(0x3C4, 0x00); port_write(0x3C5, 0x03);
    port_write(0x3C4, 0x01); port_write(0x3C5, 0x01);
    port_write(0x3C4, 0x02); port_write(0x3C5, 0x0F);
    port_write(0x3C4, 0x03); port_write(0x3C5, 0x00);
    port_write(0x3C4, 0x04); port_write(0x3C5, 0x0E);

    // Unlock CRTC registers
    port_write(0x3D4, 0x11);
    port_write(0x3D5, port_read(0x3D5) & 0x7F);

    // CRTC registers
    unsigned char crtc[] = {
        0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
        0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
        0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,0xFF
    };
    int i;
    for(i = 0; i < 25; i++)
    {
        port_write(0x3D4, i);
        port_write(0x3D5, crtc[i]);
    }

    // Graphics controller registers
    unsigned char gc[] = {
        0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF
    };
    for(i = 0; i < 9; i++)
    {
        port_write(0x3CE, i);
        port_write(0x3CF, gc[i]);
    }

    // Attribute controller registers
    port_read(0x3DA);
    unsigned char ac[] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x41,0x00,0x0F,0x00,0x00
    };
    for(i = 0; i < 21; i++)
    {
        port_write(0x3C0, i);
        port_write(0x3C0, ac[i]);
    }
    port_write(0x3C0, 0x20);
}

void draw_pixel(int x, int y, unsigned char color)
{
    // Check bounds
    if(x < 0 || x >= VGA_WIDTH)  return;
    if(y < 0 || y >= VGA_HEIGHT) return;

    unsigned char *video = (unsigned char *)VGA_MEMORY;
    video[y * VGA_WIDTH + x] = color;
}

void clear_screen_graphics(unsigned char color)
{
    unsigned char *video = (unsigned char *)VGA_MEMORY;
    int i;
    for(i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        video[i] = color;
    }
}

void draw_rect(int x, int y, int w, int h, unsigned char color)
{
    int i, j;
    for(j = y; j < y + h; j++)
    {
        for(i = x; i < x + w; i++)
        {
            draw_pixel(i, j, color);
        }
    }
}

void draw_line(int x1, int y1, int x2, int y2, unsigned char color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = dx > dy ? dx : dy;
    int i;

    for(i = 0; i <= steps; i++)
    {
        int x = x1 + (dx * i / steps);
        int y = y1 + (dy * i / steps);
        draw_pixel(x, y, color);
    }
}

void set_palette_color(unsigned char index, 
                       unsigned char r, 
                       unsigned char g, 
                       unsigned char b)
{
    port_write(0x3C8, index);      // tell VGA which color
    port_write(0x3C9, r >> 2);     // red   (6-bit)
    port_write(0x3C9, g >> 2);     // green (6-bit)
    port_write(0x3C9, b >> 2);     // blue  (6-bit)
}

void init_forerunner_palette()
{
    // Pure black
    set_palette_color(0,  0,   0,   0);
    // Deep space black (slight blue tint)
    set_palette_color(1,  5,   5,   20);
    // Forerunner gold
    set_palette_color(2,  255, 200, 50);
    // Hard light blue
    set_palette_color(3,  0,   150, 255);
    // Neon cyan
    set_palette_color(4,  0,   255, 220);
    // Verified green
    set_palette_color(5,  0,   255, 100);
    // Alert red
    set_palette_color(6,  255, 30,  30);
    // Deep blue panel
    set_palette_color(7,  10,  15,  40);
    // Pure white
    set_palette_color(8,  255, 255, 255);
    // Silver
    set_palette_color(9,  180, 180, 200);
    // Dim gold
    set_palette_color(10, 150, 120, 30);
    // Panel background
    set_palette_color(11, 8,   12,  35);
    // Scan line color
    set_palette_color(12, 3,   5,   15);
}