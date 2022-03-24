#ifndef _UI
#define _UI

#include <stdint.h>

void init_ui();
void render_ui();

void set_pixel(uint8_t x, uint8_t y, uint8_t color);

#endif