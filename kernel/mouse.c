#include "mouse.h"
#include "graphics.h"

extern screen_info_t screen;

mouse_state_t mouse;

static int prev_x = 0;
static int prev_y = 0;
static int initialized = 0;

static void outb_m(unsigned short port, unsigned char val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static unsigned char inb_m(unsigned short port)
{
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void mouse_wait_write(void)
{
    int timeout = 100000;
    while(timeout--)
    {
        if(!(inb_m(0x64) & 0x02)) return;
    }
}

static void mouse_wait_read(void)
{
    int timeout = 100000;
    while(timeout--)
    {
        if(inb_m(0x64) & 0x01) return;
    }
}

static void mouse_write(unsigned char data)
{
    mouse_wait_write();
    outb_m(0x64, 0xD4);
    mouse_wait_write();
    outb_m(0x60, data);
}

static unsigned char mouse_read(void)
{
    mouse_wait_read();
    return inb_m(0x60);
}

void mouse_init(void)
{
    mouse.x = screen.width  / 2;
    mouse.y = screen.height / 2;
    mouse.left = mouse.right = mouse.middle = 0;
    prev_x = mouse.x;
    prev_y = mouse.y;

    // Enable auxiliary mouse device
    mouse_wait_write();
    outb_m(0x64, 0xA8);

    // Read and update command byte
    mouse_wait_write();
    outb_m(0x64, 0x20);
    mouse_wait_read();
    unsigned char status = inb_m(0x60);
    status |= 0x02;
    status &= ~0x20;
    mouse_wait_write();
    outb_m(0x64, 0x60);
    mouse_wait_write();
    outb_m(0x60, status);

    // Set defaults
    mouse_write(0xF6);
    mouse_read();

    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();

    initialized = 1;
}

// Draw bold gold arrow cursor
static void draw_cursor(int x, int y, uint32_t color)
{
    // Row 0
    draw_pixel(x,   y,   color);
    draw_pixel(x+1, y,   color);
    // Row 1
    draw_pixel(x,   y+1, color);
    draw_pixel(x+1, y+1, color);
    draw_pixel(x+2, y+1, color);
    // Row 2
    draw_pixel(x,   y+2, color);
    draw_pixel(x+1, y+2, color);
    draw_pixel(x+2, y+2, color);
    draw_pixel(x+3, y+2, color);
    // Row 3
    draw_pixel(x,   y+3, color);
    draw_pixel(x+1, y+3, color);
    draw_pixel(x+2, y+3, color);
    draw_pixel(x+3, y+3, color);
    draw_pixel(x+4, y+3, color);
    // Row 4
    draw_pixel(x,   y+4, color);
    draw_pixel(x+1, y+4, color);
    draw_pixel(x+2, y+4, color);
    draw_pixel(x+3, y+4, color);
    draw_pixel(x+4, y+4, color);
    draw_pixel(x+5, y+4, color);
    // Row 5
    draw_pixel(x,   y+5, color);
    draw_pixel(x+1, y+5, color);
    draw_pixel(x+2, y+5, color);
    draw_pixel(x+3, y+5, color);
    draw_pixel(x+4, y+5, color);
    draw_pixel(x+5, y+5, color);
    draw_pixel(x+6, y+5, color);
    // Row 6
    draw_pixel(x,   y+6, color);
    draw_pixel(x+1, y+6, color);
    draw_pixel(x+2, y+6, color);
    draw_pixel(x+3, y+6, color);
    draw_pixel(x+4, y+6, color);
    draw_pixel(x+5, y+6, color);
    // Row 7
    draw_pixel(x,   y+7, color);
    draw_pixel(x+1, y+7, color);
    draw_pixel(x+2, y+7, color);
    draw_pixel(x+3, y+7, color);
    draw_pixel(x+4, y+7, color);
    // Row 8
    draw_pixel(x,   y+8, color);
    draw_pixel(x+1, y+8, color);
    draw_pixel(x+2, y+8, color);
    draw_pixel(x+3, y+8, color);
    // Row 9
    draw_pixel(x,   y+9,  color);
    draw_pixel(x+1, y+9,  color);
    draw_pixel(x+2, y+9,  color);
    // Row 10
    draw_pixel(x,   y+10, color);
    draw_pixel(x+1, y+10, color);
    // Row 11
    draw_pixel(x,   y+11, color);
}

static void erase_cursor(int x, int y)
{
    int i, j;
    for(j = 0; j <= 12; j++)
        for(i = 0; i <= 8; i++)
            draw_pixel(x+i, y+j, 0x050514);
}

void mouse_poll(void)
{
    if(!initialized) return;

    unsigned char status = inb_m(0x64);
    if(!(status & 0x01)) return;
    if(!(status & 0x20)) return;

    unsigned char byte1 = inb_m(0x60);
    if(!(byte1 & 0x08)) return;

    status = inb_m(0x64);
    if(!(status & 0x01)) return;
    unsigned char byte2 = inb_m(0x60);

    status = inb_m(0x64);
    if(!(status & 0x01)) return;
    unsigned char byte3 = inb_m(0x60);

    // Parse movement
    int dx = (int)byte2;
    int dy = (int)byte3;

    // Sign extension
    if(byte1 & 0x10) dx |= 0xFFFFFF00;
    if(byte1 & 0x20) dy |= 0xFFFFFF00;

    // Overflow clamp
    if(byte1 & 0x40) dx = 0;
    if(byte1 & 0x80) dy = 0;

    // Update position (Y inverted)
    mouse.x += dx;
    mouse.y -= dy;

    // Clamp to screen bounds
    if(mouse.x < 0) mouse.x = 0;
    if(mouse.y < 0) mouse.y = 0;
    if(mouse.x >= (int)screen.width  - 12) mouse.x = screen.width  - 12;
    if(mouse.y >= (int)screen.height - 12) mouse.y = screen.height - 12;

    // Buttons
    mouse.left   = (byte1 & 0x01) ? 1 : 0;
    mouse.right  = (byte1 & 0x02) ? 1 : 0;
    mouse.middle = (byte1 & 0x04) ? 1 : 0;

    if(mouse.x != prev_x || mouse.y != prev_y)
    {
        erase_cursor(prev_x, prev_y);

        // Black shadow for depth
        draw_cursor(mouse.x+1, mouse.y+1, 0x000000);
        // Gold cursor
        draw_cursor(mouse.x,   mouse.y,   0xFFC832);

        prev_x = mouse.x;
        prev_y = mouse.y;
    }

    // Flash brighter gold on left click
    if(mouse.left)
    {
        draw_cursor(mouse.x, mouse.y, 0xFFE87A);
    }
}

mouse_state_t mouse_get(void)
{
    return mouse;
}