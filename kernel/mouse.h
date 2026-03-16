#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x;
    int y;
    int left;
    int right;
    int middle;
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);
mouse_state_t mouse_get(void);

extern mouse_state_t mouse;

#endif