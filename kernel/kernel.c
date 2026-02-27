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

void kernel_main(void)
{
    // Switch to graphics mode
    graphics_init();

    // Clear screen to black
    clear_screen_graphics(COLOR_BLACK);

    // Draw Sentinel boot screen
    // Gold bar at top
    draw_rect(0, 0, 320, 10, COLOR_GOLD);

    // Gold bar at bottom
    draw_rect(0, 190, 320, 10, COLOR_GOLD);

    // Draw centered rectangle - main panel
    draw_rect(60, 30, 200, 140, COLOR_DARKGRAY);

    // Draw gold border around panel
    draw_line(60, 30, 260, 30, COLOR_GOLD);
    draw_line(60, 170, 260, 170, COLOR_GOLD);
    draw_line(60, 30, 60, 170, COLOR_GOLD);
    draw_line(260, 30, 260, 170, COLOR_GOLD);

    while(1) {}
}