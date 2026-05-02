#include "timer.h"
#include "scheduler.h"
#include "idt.h"

// Actual definition
unsigned int timer_ticks = 0;

// Write to port
static void port_write(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Send EOI to master PIC
static void pic_send_eoi(void)
{
    port_write(0x20, 0x20);
}

// Called from irq0_handler in irq.asm
void timer_handler_c(void)
{
    timer_ticks++;

    // Send EOI — must happen every IRQ
    pic_send_eoi();

    // Round Robin tick
    schedule_tick();
}

unsigned int timer_get_ticks(void)
{
    return timer_ticks;
}

unsigned int timer_get_seconds(void)
{
    return timer_ticks / PIT_FREQUENCY;
}

void timer_init(void)
{
    // Initialize PIC (8259A)
    port_write(0x20, 0x11);   // ICW1
    port_write(0xA0, 0x11);
    port_write(0x21, 0x20);   // ICW2 — IRQ0 maps to INT 32
    port_write(0xA1, 0x28);
    port_write(0x21, 0x04);   // ICW3
    port_write(0xA1, 0x02);
    port_write(0x21, 0x01);   // ICW4
    port_write(0xA1, 0x01);
    port_write(0x21, 0x00);   // unmask all IRQs
    port_write(0xA1, 0x00);

    // Set PIT to 100Hz
    unsigned int divisor = 1193180 / PIT_FREQUENCY;
    port_write(PIT_COMMAND,  0x36);
    port_write(PIT_CHANNEL0, divisor & 0xFF);
    port_write(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    // Wire IRQ0 to ASM stub
    extern void irq0_handler(void);
    idt_set_entry(32, (uint64_t)irq0_handler, 0x08, 0x8E);

    // Enable interrupts
    

    timer_ticks = 0;
}

void timer_wait(unsigned int ticks)
{
    unsigned int start = timer_ticks;
    while(timer_ticks < start + ticks) {}
}