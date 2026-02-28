#ifndef FONT_H
#define FONT_H

// Each character is 8x8 pixels = 8 bytes
#define FONT_WIDTH  8
#define FONT_HEIGHT 8

// Font data - 128 characters x 8 bytes each
extern unsigned char font_data[128][8];

// Function signatures
void draw_char(int x, int y, char c, unsigned char color);
void draw_string(int x, int y, const char *str, unsigned char color);

#endif