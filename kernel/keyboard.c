#include "keyboard.h"

// Scancode to ASCII lookup table
static char scancode_table[58] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Last key pressed
static char last_key = 0;

// Read a byte from a hardware port
static unsigned char port_read(unsigned short port)
{
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void keyboard_handler()
{
    unsigned char scancode = port_read(KEYBOARD_DATA_PORT);

    // Only handle key press (not release)
    // Key release scancodes have bit 7 set
    if(scancode & 0x80)
        return;

    if(scancode < 58)
        last_key = scancode_table[scancode];
}

char keyboard_getchar()
{
    char key = last_key;
    last_key = 0;
    return key;
}

void keyboard_init()
{
    // Register keyboard handler in IDT slot 33
    // IRQ1 = interrupt 33 (32 + 1)
    last_key = 0;
}