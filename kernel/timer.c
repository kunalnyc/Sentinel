#include "timer.h"

// Actual definition
unsigned int timer_ticks = 0;

// Write to port
static void port_write(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}


void timer_handler()
{
    timer_ticks++;
}

unsigned int timer_get_ticks()
{
    return timer_ticks;
}

unsigned int timer_get_seconds()
{
    return timer_ticks / PIT_FREQUENCY;
}

void timer_init()
{
    // Initialize PIC first
    port_write(0x20, 0x11);
    port_write(0xA0, 0x11);
    port_write(0x21, 0x20);
    port_write(0xA1, 0x28);
    port_write(0x21, 0x04);
    port_write(0xA1, 0x02);
    port_write(0x21, 0x01);
    port_write(0xA1, 0x01);
    port_write(0x21, 0x00);
    port_write(0xA1, 0x00);

    // Set PIT frequency
    unsigned int divisor = 1193180 / PIT_FREQUENCY;
    port_write(PIT_COMMAND, 0x36);
    port_write(PIT_CHANNEL0, divisor & 0xFF);
    port_write(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    // Register timer handler in IDT
    extern void idt_set_entry(int n, unsigned int handler);
    idt_set_entry(32, (unsigned long long)timer_handler);

    // Enable interrupts
    __asm__ volatile("sti");

    timer_ticks = 0;
}
void timer_wait(unsigned int ticks)
{
    unsigned int start = timer_ticks;
    while(timer_ticks < start + ticks) {}
}