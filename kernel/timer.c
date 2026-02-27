#include "timer.h"

// Actual definition
unsigned int timer_ticks = 0;

// Write byte to hardware port
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
    // Calculate divisor for 100Hz
    unsigned int divisor = 1193180 / PIT_FREQUENCY;

    // Send command to PIT
    port_write(PIT_COMMAND, 0x36);

    // Send divisor low byte first then high byte
    port_write(PIT_CHANNEL0, divisor & 0xFF);
    port_write(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    timer_ticks = 0;
}