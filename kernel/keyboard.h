#ifndef KEYBOARD_H
#define KEYBOARD_H

// Keyboard data port
#define KEYBOARD_DATA_PORT 0x60

// Function signatures
void keyboard_init();
char keyboard_getchar();
char keyboard_poll(void);
void keyboard_handler();

#endif