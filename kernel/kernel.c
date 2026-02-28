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

void kernel_main(void)
{
   graphics_init();
    clear_screen_graphics(COLOR_BLACK);

    // Gold bars top and bottom
    draw_rect(0, 0, 320, 10, COLOR_GOLD);
    draw_rect(0, 190, 320, 10, COLOR_GOLD);

    // Main panel
    draw_rect(60, 30, 200, 140, COLOR_DARKGRAY);

    // Gold border
    draw_line(60, 30, 260, 30, COLOR_GOLD);
    draw_line(60, 170, 260, 170, COLOR_GOLD);
    draw_line(60, 30, 60, 170, COLOR_GOLD);
    draw_line(260, 30, 260, 170, COLOR_GOLD);

    // Draw text on screen!
    draw_string(80, 40, "SENTINELOS", COLOR_GOLD);
    draw_string(80, 55, "VERSION 0.1", COLOR_WHITE);
    draw_string(80, 80, "TRUST REGISTRY ONLINE", COLOR_GREEN);
    draw_string(80, 95, "MEMORY MANAGER ONLINE", COLOR_GREEN);
    draw_string(80, 110, "SHA-256 ENGINE ONLINE", COLOR_GREEN);
    draw_string(80, 125, "VERIFICATION GATE ACTIVE", COLOR_GREEN);
    draw_string(70, 155, "ALL SYSTEMS OPERATIONAL", COLOR_GOLD);

    while(1) {}
}