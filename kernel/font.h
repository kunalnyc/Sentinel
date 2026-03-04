#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 8

// Match the graphics.h signature
void draw_char(char c, int x, int y, uint32_t color, uint32_t bgcolor);
void draw_string(int x, int y, const char *str, uint32_t color);
void draw_string_with_bg(int x, int y, const char *str, uint32_t color, uint32_t bgcolor);

// Font data
extern uint8_t font_8x8_basic[256][8];

#endif