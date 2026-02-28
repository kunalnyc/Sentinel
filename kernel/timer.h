#ifndef TIMER_H
#define TIMER_H

// PIT ports
#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQUENCY 100  // 100Hz = 10ms per tick

// Global tick counter
extern unsigned int timer_ticks;

// Function signatures
void timer_init();
void timer_handler();
unsigned int timer_get_ticks();
unsigned int timer_get_seconds();

void timer_wait(unsigned int seconds);

#endif