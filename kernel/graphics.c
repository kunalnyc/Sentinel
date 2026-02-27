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