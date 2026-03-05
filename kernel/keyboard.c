#include "keyboard.h"
#include "idt.h"

static char scancode_table[58] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static char last_key = 0;

static unsigned char port_read(unsigned short port)
{
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void port_write(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void keyboard_handler()
{
    unsigned char scancode = port_read(KEYBOARD_DATA_PORT);

    if(scancode & 0x80) {
        // Send EOI
        port_write(0x20, 0x20);
        return;
    }

    if(scancode == 0x1C) { last_key = '\n'; }
    else if(scancode == 0x01) { last_key = 27; }
    else if(scancode == 0x0E) { last_key = '\b'; }
    else if(scancode == 0x39) { last_key = ' '; }
    else if(scancode < 58) {
        char c = scancode_table[scancode];
        if(c) last_key = c;
    }

    // Send EOI to PIC
    port_write(0x20, 0x20);
}

char keyboard_getchar()
{
    char key = last_key;
    last_key = 0;
    return key;
}

// Add this to keyboard.c at the bottom
char keyboard_poll()
{
    static unsigned char last_scancode = 0;
    
    unsigned char status = port_read(0x64);
    if(!(status & 0x01)) return 0;
    
    unsigned char scancode = port_read(0x60);
    
    if(scancode & 0x80) { last_scancode = 0; return 0; }
    if(scancode == last_scancode) return 0;
    last_scancode = scancode;
    
    if(scancode == 0x1C) return '\n';
    if(scancode == 0x01) return 27;
    if(scancode == 0x0E) return '\b';
    if(scancode == 0x39) return ' ';
    if(scancode < 58) return scancode_table[scancode];
    
    return 0;
}
// Keyboard handler wrapper in assembly
extern void keyboard_handler_asm(void);
__asm__(
    ".global keyboard_handler_asm\n"
    "keyboard_handler_asm:\n"
    "    push %rax\n"
    "    push %rcx\n"
    "    push %rdx\n"
    "    push %rsi\n"
    "    push %rdi\n"
    "    push %r8\n"
    "    push %r9\n"
    "    push %r10\n"
    "    push %r11\n"
    "    call keyboard_handler\n"
    "    pop %r11\n"
    "    pop %r10\n"
    "    pop %r9\n"
    "    pop %r8\n"
    "    pop %rdi\n"
    "    pop %rsi\n"
    "    pop %rdx\n"
    "    pop %rcx\n"
    "    pop %rax\n"
    "    iretq\n"
);

void keyboard_init()
{
    // Register IRQ1 handler in IDT slot 33
    idt_set_entry(33, (uint64_t)keyboard_handler_asm, 0x08, 0x8E);

    // Initialize IDT
    idt_init();

    // Enable interrupts
    __asm__ volatile("sti");

    last_key = 0;
}