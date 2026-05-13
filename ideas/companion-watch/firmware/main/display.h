#pragma once
#include <stdint.h>

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

void display_init(void);
void display_clear(uint16_t color);
void display_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *data);
